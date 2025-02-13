/*
** Copyright (C) 2020-2022 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
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

#ifndef __MU_CONTACTS_CACHE_HH__
#define __MU_CONTACTS_CACHE_HH__

#include <glib.h>
#include <time.h>
#include <memory>
#include <functional>
#include <chrono>
#include <string>
#include <time.h>
#include <inttypes.h>
#include <utils/mu-utils.hh>

#include <message/mu-message.hh>

namespace Mu {

class ContactsCache {
public:
	/**
	 * Construct a new ContactsCache object
	 *
	 * @param serialized serialized contacts
	 * @param personal personal addresses
	 */
	ContactsCache(const std::string& serialized = "", const StringVec& personal = {});

	/**
	 * DTOR
	 *
	 */
	~ContactsCache();

	/**
	 * Add a contact
	 *
	 * @param contact a Contact object
	 *
	 */
	void add(Contact&& contact);


	/**
	 * Add a contacts sequemce
	 *
	 * @param contacts a Contact object sequence
	 *
	 */
	void add(Contacts&& contacts) {
		for (auto&& contact: contacts)
			add(std::move(contact));
	}

	/**
	 * Clear all contacts
	 *
	 */
	void clear();

	/**
	 * Get the number of contacts
	 *

	 * @return number of contacts
	 */
	std::size_t size() const;

	/**
	 * Are there no contacts?
	 *
	 * @return true or false
	 */
	bool empty() const { return size() == 0; }

	/**
	 * Get the contacts, serialized. This all marks the data as
	 * non-dirty (see dirty())
	 *
	 * @return serialized contacts
	 */
	std::string serialize() const;

	/**
	 * Has the contacts database change since the last
	 * call to serialize()?
	 *
	 * @return true or false
	 */
	bool dirty() const;

	/**
	 * Does this look like a 'personal' address?
	 *
	 * @param addr some e-mail address
	 *
	 * @return true or false
	 */
	bool is_personal(const std::string& addr) const;

	/**
	 * Find a contact based on the email address. This is not safe, since
	 * the returned ptr can be invalidated at any time; only for unit-tests.
	 *
	 * @param email email address
	 *
	 * @return contact info, or {} if not found
	 */
	const Contact* _find(const std::string& email) const;

	/**
	 * Prototype for a callable that receives a contact
	 *
	 * @param contact some contact
	 */
	using EachContactFunc = std::function<void(const Contact& contact_info)>;

	/**
	 * Invoke some callable for each contact, in order of rank.
	 *
	 * @param each_contact
	 */
	void for_each(const EachContactFunc& each_contact) const;

private:
	struct Private;
	std::unique_ptr<Private> priv_;
};

} // namespace Mu

#endif /* __MU_CONTACTS_CACHE_HH__ */
