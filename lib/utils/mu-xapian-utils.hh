/*
** Copyright (C) 2021 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
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

#ifndef MU_XAPIAN_UTILS_HH__
#define MU_XAPIAN_UTILS_HH__

#include <xapian.h>
#include <glib.h>

namespace Mu {

// avoid exception-handling boilerplate.
template <typename Func>
void
xapian_try(Func&& func) noexcept
try {
	func();
} catch (const Xapian::Error& xerr) {
	g_critical("%s: xapian error '%s'", __func__, xerr.get_msg().c_str());
} catch (const std::runtime_error& re) {
	g_critical("%s: error: %s", __func__, re.what());
} catch (const std::exception& e) {
	g_critical("%s: caught exception: %s", __func__, e.what());
} catch (...) {
	g_critical("%s: caught exception", __func__);
}

template <typename Func, typename Default = std::invoke_result<Func>>
auto
xapian_try(Func&& func, Default&& def) noexcept -> std::decay_t<decltype(func())>
try {
	return func();
} catch (const Xapian::Error& xerr) {
	g_critical("%s: xapian error '%s'", __func__, xerr.get_msg().c_str());
	return static_cast<Default>(def);
} catch (const std::runtime_error& re) {
	g_critical("%s: error: %s", __func__, re.what());
	return static_cast<Default>(def);
} catch (const std::exception& e) {
	g_critical("%s: caught exception: %s", __func__, e.what());
	return static_cast<Default>(def);
} catch (...) {
	g_critical("%s: caught exception", __func__);
	return static_cast<Default>(def);
}

} // namespace Mu

#endif /* MU_ XAPIAN_UTILS_HH__ */
