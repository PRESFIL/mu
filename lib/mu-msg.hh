/*
** Copyright (C) 2010-2022 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#ifndef MU_MSG_HH__
#define MU_MSG_HH__

#include <utils/mu-result.hh>

#include <message/mu-message.hh>

#include <utils/mu-util.h>
#include <utils/mu-utils.hh>
#include <utils/mu-option.hh>
#include <utils/mu-sexp.hh>

namespace Mu {

struct MuMsg;

/* options for various functions */
enum MuMsgOptions {
	MU_MSG_OPTION_NONE = 0,
	/* 1 << 0 is still free! */

	/* for -> sexp conversion */
	MU_MSG_OPTION_HEADERS_ONLY   = 1 << 1,
	MU_MSG_OPTION_EXTRACT_IMAGES = 1 << 2,

	/* below options are for checking signatures; only effective
	 * if mu was built with crypto support */
	MU_MSG_OPTION_VERIFY        = 1 << 4,
	MU_MSG_OPTION_AUTO_RETRIEVE = 1 << 5,
	MU_MSG_OPTION_USE_AGENT     = 1 << 6,
	/* MU_MSG_OPTION_USE_PKCS7         = 1 << 7,   /\* gpg is the default *\/ */

	/* get password from console if needed */
	MU_MSG_OPTION_CONSOLE_PASSWORD = 1 << 7,

	MU_MSG_OPTION_DECRYPT = 1 << 8,

	/* misc */
	MU_MSG_OPTION_OVERWRITE    = 1 << 9,
	MU_MSG_OPTION_USE_EXISTING = 1 << 10,

	/* recurse into submessages */
	MU_MSG_OPTION_RECURSE_RFC822 = 1 << 11
};
MU_ENABLE_BITOPS(MuMsgOptions);

/**
 * create a new MuMsg* instance which parses a message and provides
 * read access to its properties; call mu_msg_unref when done with it.
 *
 * @param path full path to an email message file
 * @param mdir the maildir for this message; ie, if the path is
 * ~/Maildir/foo/bar/cur/msg, the maildir would be foo/bar; you can
 * pass NULL for this parameter, in which case some maildir-specific
 * information is not available.
 * @param err receive error information (MU_ERROR_FILE or
 * MU_ERROR_GMIME), or NULL. There will only be err info if the
 * function returns NULL
 *
 * @return a new MuMsg instance or NULL in case of error; call
 * mu_msg_unref when done with this message
 */
MuMsg* mu_msg_new_from_file(const char* filepath,
			    const char* maildir,
			    GError**    err) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

/**
 * create a new MuMsg* instance based on a Xapian::Document
 *
 * @param store a MuStore ptr
 * @param doc a ptr to a Xapian::Document (but cast to XapianDocument,
 * because this is C not C++). MuMsg takes _ownership_ of this pointer;
 * don't touch it afterwards
 * @param err receive error information, or NULL. There
 * will only be err info if the function returns NULL
 *
 * @return a new MuMsg instance or NULL in case of error; call
 * mu_msg_unref when done with this message
 */
MuMsg* mu_msg_new_from_doc(XapianDocument* doc,
			   GError**        err) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

/**
 *  if we don't have a message file yet (because this message is
 *  database-backed), load it.
 *
 * @param msg a MuMsg
 * @param err receives error information
 *
 * @return TRUE if this succeeded, FALSE in case of error
 */
gboolean mu_msg_load_msg_file(MuMsg* msg, GError** err);

/**
 * close the file-backend, if any; this function is for the use case
 * where you have a large amount of messages where you need some
 * file-backed field (body or attachments). If you don't close the
 * file-backend after retrieving the desired field, you'd quickly run
 * out of file descriptors. If this message does not have a
 * file-backend, do nothing.
 *
 * @param msg a message object
 */
void mu_msg_unload_msg_file(MuMsg* msg);

/**
 * increase the reference count for this message
 *
 * @param msg a message
 *
 * @return the message with its reference count increased, or NULL in
 * case of error.
 */
MuMsg* mu_msg_ref(MuMsg* msg);

/**
 * decrease the reference count for this message. if the reference
 * count reaches 0, the message will be destroyed.
 *
 * @param msg a message
 */
void mu_msg_unref(MuMsg* msg);

/**
 * cache the values from the backend (file or db), so we don't the
 * backend anymore
 *
 * @param self a message
 */
void mu_msg_cache_values(MuMsg* self);

/**
 * get the plain text body of this message
 *
 * @param msg a valid MuMsg* instance
 * @param opts options for getting the body
 *
 * @return the plain text body or NULL in case of error or if there is no
 * such body. the returned string should *not* be modified or freed.
 * The returned data is in UTF8 or NULL.
 */
const char* mu_msg_get_body_text(MuMsg* msg, MuMsgOptions opts);

/**
 * get the content type parameters for the text body part
 *
 * @param msg a valid MuMsg* instance
 * @param opts options for getting the body
 *
 * @return the value of the requested body part content type parameter, or
 * NULL in case of error or if there is no such body. the returned string
 * should *not* be modified or freed. The returned data is in UTF8 or NULL.
 */
const GSList* mu_msg_get_body_text_content_type_parameters(MuMsg* self, MuMsgOptions opts);

/**
 * get the html body of this message
 *
 * @param msg a valid MuMsg* instance
 * @param opts options for getting the body
 *
 * @return the html body or NULL in case of error or if there is no
 * such body. the returned string should *not* be modified or freed.
 */
const char* mu_msg_get_body_html(MuMsg* msgMu, MuMsgOptions opts);

/**
 * get the sender (From:) of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the sender of this Message or NULL in case of error or if there
 * is no sender. the returned string should *not* be modified or freed.
 */
const char* mu_msg_get_from(MuMsg* msg);

/**
 * get the recipients (To:) of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the sender of this Message or NULL in case of error or if there
 * are no recipients. the returned string should *not* be modified or freed.
 */
const char* mu_msg_get_to(MuMsg* msg);

/**
 * get the carbon-copy recipients (Cc:) of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the Cc: recipients of this Message or NULL in case of error or if
 * there are no such recipients. the returned string should *not* be modified
 * or freed.
 */
const char* mu_msg_get_cc(MuMsg* msg);

/**
 * get the blind carbon-copy recipients (Bcc:) of this message; this
 * field usually only appears in outgoing messages
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the Bcc: recipients of this Message or NULL in case of
 * error or if there are no such recipients. the returned string
 * should *not* be modified or freed.
 */
const char* mu_msg_get_bcc(MuMsg* msg);

/**
 * get the file system path of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the path of this Message or NULL in case of error.
 * the returned string should *not* be modified or freed.
 */
const char* mu_msg_get_path(MuMsg* msg);

/**
 * get the maildir this message lives in; ie, if the path is
 * ~/Maildir/foo/bar/cur/msg, the maildir would be foo/bar
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the maildir requested or NULL in case of error. The returned
 * string should *not* be modified or freed.
 */
const char* mu_msg_get_maildir(MuMsg* msg);

/**
 * get the subject of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the subject of this Message or NULL in case of error or if there
 * is no subject. the returned string should *not* be modified or freed.
 */
const char* mu_msg_get_subject(MuMsg* msg);

/**
 * get the Message-Id of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the Message-Id of this message (without the enclosing <>),
 * or a fake message-id for messages that don't have them, or NULL in
 * case of error.
 */
const char* mu_msg_get_msgid(MuMsg* msg);

/**
 * get the mailing list for a message, i.e. the mailing-list
 * identifier in the List-Id header.
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the mailing list id for this message (without the enclosing <>)
 * or NULL in case of error or if there is none. the returned string
 * should *not* be modified or freed.
 */
const char* mu_msg_get_mailing_list(MuMsg* msg);

/**
 * get the message date/time (the Date: field) as time_t, using UTC
 *
 * @param msg a valid MuMsg* instance
 *
 * @return message date/time or 0 in case of error or if there
 * is no such header.
 */
time_t mu_msg_get_date(MuMsg* msg);

/**
 * get the flags for this message
 *
 * @param msg valid MuMsg* instance
 *
 * @return the file/content flags as logically OR'd #Mu::Flags.
 * Non-standard flags are ignored.
 */
Flags mu_msg_get_flags(MuMsg* msg);

/**
 * get the file size in bytes of this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the filesize
 */
size_t mu_msg_get_size(MuMsg* msg);

/**
 * get some field value as string
 *
 * @param msg a valid MuMsg instance
 * @param field the field to retrieve; it must be a string-typed field
 *
 * @return a string that should not be freed
 */
const char* mu_msg_get_field_string(MuMsg* msg, Field::Id mfid);

/**
 * get some field value as string-list
 *
 * @param msg a valid MuMsg instance
 * @param field the field to retrieve; it must be a string-list-typed field
 *
 * @return a list that should not be freed
 */
const GSList* mu_msg_get_field_string_list(MuMsg* self, Field::Id mfid);

/**
 * get some field value as string
 *
 * @param msg a valid MuMsg instance
 * @param field the field to retrieve; it must be a numeric field
 *
 * @return a string that should not be freed
 */
gint64 mu_msg_get_field_numeric(MuMsg* msg, Field::Id mfid);

/**
 * get the message priority for this message. The X-Priority, X-MSMailPriority,
 * Importance and Precedence header are checked, in that order. if no known or
 * explicit priority is set, MessagePriority::Id::Normal is assumed
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the message priority
 */
Priority mu_msg_get_prio(MuMsg* msg);

/**
 * get the timestamp (mtime) for the file containing this message
 *
 * @param msg a valid MuMsg* instance
 *
 * @return the timestamp or 0 in case of error
 */
time_t mu_msg_get_timestamp(MuMsg* msg);

/**
 * get a specific header from the message. This value will _not_ be
 * cached
 *
 * @param self a MuMsg instance
 * @param header a specific header (like 'X-Mailer' or 'Organization')
 *
 * @return a header string which is valid as long as this MuMsg is
 */
const char* mu_msg_get_header(MuMsg* self, const char* header);

/**
 * get the list of references (consisting of both the References and
 * In-Reply-To fields), with the oldest first and the direct parent as
 * the last one. Note, any reference (message-id) will appear at most
 * once, duplicates are filtered out.
 *
 * @param msg a valid MuMsg
 *
 * @return a list with the references for this msg. Don't modify/free
 */
const GSList* mu_msg_get_references(MuMsg* msg);

/**
 * get the list of tags (ie., X-Label)
 *
 * @param msg a valid MuMsg
 *
 * @return a list with the tags for this msg. Don't modify/free
 */
const GSList* mu_msg_get_tags(MuMsg* self);

/**
 * compare two messages for sorting
 *
 * @param m1 a message
 * @param m2 another message
 * @param mfid the message to use for the comparison
 *
 * @return negative if m1 is smaller, positive if m1 is smaller, 0 if
 * they are equal
 */
int mu_msg_cmp(MuMsg* m1, MuMsg* m2, Field::Id mfid);

/**
 * check whether there there's a readable file behind this message
 *
 * @param self a MuMsg*
 *
 * @return TRUE if the message file is readable, FALSE otherwise
 */
gboolean mu_msg_is_readable(MuMsg* self);

/**
 * move a message to another maildir; note that this does _not_ update
 * the database
 *
 * @param msg a message with an existing file system path in an actual
 * maildir
 * @param root_maildir_path file system path for the root-maildir for this message
 * e.g., /home/user/Maildir
 * @param target_maildir the subdir where the message should go, relative to
 * rootmaildir. e.g. "/archive"
 * @param flags to set for the target (influences the filename, path)
 * @param silently ignore the src=target case (return TRUE)
 * @param new_name whether to create a new unique name, or keep the
 * old one
 * @param err receives error information
 *
 * @return TRUE if it worked, FALSE otherwise
 */
bool mu_msg_move_to_maildir(MuMsg*		msg,
			    const std::string&	root_maildir_path,
			    const std::string&	target_maildir,
			    Flags		flags,
			    bool		ignore_dups,
			    bool		new_name,
			    GError**		err);
/**
 * Get a sequence with contacts of the given type for this message.
 *
 * @param msg a valid MuMsg* instance
 * @param field_id the contact field or none; if none get _all_ contact types.
 *
 * @return a sequence
 */
Mu::Contacts mu_msg_get_contacts (MuMsg *self,
				  std::optional<Field::Id> field_id={});
/**
 * create a 'display contact' from an email header To/Cc/Bcc/From-type address
 * ie., turn
 *     "Foo Bar" <foo@bar.com>
 * into
 *      Foo Bar
 * Note that this is based on some simple heuristics. Max length is 255 bytes.
 *
 *   mu_str_display_contact_s returns a statically allocated
 *   buffer (ie, non-reentrant), while mu_str_display_contact
 *   returns a newly allocated string that you must free with g_free
 *   when done with it.
 *
 * @param str a 'contact str' (ie., what is in the To/Cc/Bcc/From
 * fields), or NULL
 *
 * @return a newly allocated string with a display contact
 */
const char* mu_str_display_contact_s(const char* str) G_GNUC_CONST;
char*       mu_str_display_contact(const char* str) G_GNUC_WARN_UNUSED_RESULT;



struct QueryMatch;

/**
 * convert the msg to a Lisp symbolic expression (for further processing in
 * e.g. emacs)
 *
 * @param msg a valid message
 * @param docid the docid for this message, or 0
 * @param opts, bitwise OR'ed;
 *    - MU_MSG_OPTION_HEADERS_ONLY: only include message fields which can be
 *      obtained from the database (this is much faster if the MuMsg is
 *      database-backed, so no file needs to be opened)
 *    - MU_MSG_OPTION_EXTRACT_IMAGES: extract image attachments as temporary
 *      files and include links to those in the sexp
 *  and for message parts:
 *	MU_MSG_OPTION_CHECK_SIGNATURES: check signatures
 *	MU_MSG_OPTION_AUTO_RETRIEVE_KEY: attempt to retrieve keys online
 *	MU_MSG_OPTION_USE_AGENT: attempt to use GPG-agent
 *	MU_MSG_OPTION_USE_PKCS7: attempt to use PKCS (instead of gpg)
 *
 * @return a Mu::Sexp or a Mu::Sexp::List representing the message.
 */
Mu::Sexp::List msg_to_sexp_list(MuMsg* msg, unsigned docid, MuMsgOptions ops);
Mu::Sexp       msg_to_sexp(MuMsg* msg, unsigned docid, MuMsgOptions ops);
} // namespace Mu

#endif /*MU_MSG_HH__*/
