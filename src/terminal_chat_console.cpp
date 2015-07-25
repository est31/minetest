/*
Minetest
Copyright (C) 2015 est31 <MTest31@outlook.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "cmake_config.h"
#if USE_CURSES
#include "version.h"
#include "terminal_chat_console.h"
#include "porting.h"
#include "settings.h"
#include "util/numeric.h"
#include "util/string.h"

// include this last to avoid any conflicts
// (likes to set macros to common names, conflicting various stuff)
#include "ncursesw/curses.h"


// Some functions to make drawing etc position independent
static void reformat_backend(ChatBackend *backend, int rows, int cols)
{
	backend->reformat(cols, rows - 2);
}

static void move_for_backend(int row, int col)
{
	move(row + 1, col);
}

void TerminalChatConsole::init()
{
	initscr();
	cbreak(); //raw();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	timeout(100);

	// To make esc not delay up to one second. This is the value vim uses, too.
	ESCDELAY = 25;

	getmaxyx(stdscr, m_rows, m_cols);
	reformat_backend(&m_chat_backend, m_rows, m_cols);
}

void TerminalChatConsole::deinit()
{
	endwin();
}

void TerminalChatConsole::resize()
{
	m_chat_backend.reformat(m_cols, m_rows - 2);
}

void *TerminalChatConsole::run()
{
	DSTACK(__FUNCTION_NAME);
	BEGIN_DEBUG_EXCEPTION_HANDLER

	// Inform the server of our nick
	m_chat_interface->command_queue.push_back(
		ChatEvent(CET_NICK_ADD, m_nick, L""));

	while (!stopRequested()) {

		int ch = getch();
		if (stopRequested())
			break;

		step(ch);
	}

	m_kill_requested = true;

	END_DEBUG_EXCEPTION_HANDLER(errorstream)

	return NULL;
}

void TerminalChatConsole::typeChatMessage(const std::wstring &m)
{
	// Discard empty line
	if (m == L"")
		return;

	// Send to server
	m_chat_interface->command_queue.push_back(
		ChatEvent(CET_CHAT, m_nick, m));

	// Print if its a command (gets eaten by server otherwise)
	if (m[0] == L'/') {
		m_chat_backend.addMessage(L"", (std::wstring)L"Issued command: " + m);
	}
}

void TerminalChatConsole::handleInput(int ch, bool &complete_redraw_needed)
{
	switch (ch) {
		case ERR: // no input
			break;
		case 27: // ESC
			// Toggle ESC mode
			m_esc_mode = !m_esc_mode;
			break;
		case KEY_PPAGE:
			m_chat_backend.scrollPageUp();
			complete_redraw_needed = true;
			break;
		case KEY_NPAGE:
			m_chat_backend.scrollPageDown();
			complete_redraw_needed = true;
			break;
		case KEY_ENTER:
		case '\r':
		case '\n': {
			std::wstring text = m_chat_backend.getPrompt().submit();
			typeChatMessage(text);
			break;
		} case KEY_UP:
			m_chat_backend.getPrompt().historyPrev();
			break;
		case KEY_DOWN:
			m_chat_backend.getPrompt().historyNext();
			break;
		case KEY_LEFT:
			// Left pressed
			// move character to the left
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_MOVE,
				ChatPrompt::CURSOROP_DIR_LEFT,
				ChatPrompt::CURSOROP_SCOPE_CHARACTER);
			break;
		case KEY_RIGHT:
			// Right pressed
			// move character to the right
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_MOVE,
				ChatPrompt::CURSOROP_DIR_RIGHT,
				ChatPrompt::CURSOROP_SCOPE_CHARACTER);
			break;
		case KEY_HOME:
			// Home pressed
			// move to beginning of line
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_MOVE,
				ChatPrompt::CURSOROP_DIR_LEFT,
				ChatPrompt::CURSOROP_SCOPE_LINE);
			break;
		case KEY_END:
			// End pressed
			// move to end of line
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_MOVE,
				ChatPrompt::CURSOROP_DIR_RIGHT,
				ChatPrompt::CURSOROP_SCOPE_LINE);
			break;
		case KEY_BACKSPACE:
		case '\b':
			// Backspace pressed
			// delete character to the left
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_DELETE,
				ChatPrompt::CURSOROP_DIR_LEFT,
				ChatPrompt::CURSOROP_SCOPE_CHARACTER);
			break;
		case KEY_DC:
			// Delete pressed
			// delete character to the right
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_DELETE,
				ChatPrompt::CURSOROP_DIR_RIGHT,
				ChatPrompt::CURSOROP_SCOPE_CHARACTER);
			break;
		case 21:
			// Ctrl-U pressed
			// kill line to left end
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_DELETE,
				ChatPrompt::CURSOROP_DIR_LEFT,
				ChatPrompt::CURSOROP_SCOPE_LINE);
			break;
		case 11:
			// Ctrl-K pressed
			// kill line to right end
			m_chat_backend.getPrompt().cursorOperation(
				ChatPrompt::CURSOROP_DELETE,
				ChatPrompt::CURSOROP_DIR_RIGHT,
				ChatPrompt::CURSOROP_SCOPE_LINE);
			break;
		case KEY_TAB:
			// Tab pressed
			// Nick completion
			m_chat_backend.getPrompt().nickCompletion(m_nicks, false);
			break;
		default:
			// Add character to the prompt,
			// assuming UTF-8.
			if (IS_UTF8_MULTB_START(ch)) {
				m_pending_utf8_bytes.append(1, (char)ch);
				m_utf8_bytes_to_wait += UTF8_MULTB_START_LEN(ch) - 1;
			} else if (m_utf8_bytes_to_wait != 0) {
				m_pending_utf8_bytes.append(1, (char)ch);
				m_utf8_bytes_to_wait--;
				if (m_utf8_bytes_to_wait == 0) {
					std::wstring w = utf8_to_wide(m_pending_utf8_bytes);
					m_pending_utf8_bytes = "";
					// hopefully only one char in the wstring...
					for (size_t i = 0; i < w.size(); i++) {
						m_chat_backend.getPrompt().input(w.c_str()[i]);
					}
				}
			} else {
				m_chat_backend.getPrompt().input(ch);
			}
			break;
	}
}

void TerminalChatConsole::step(int ch)
{
	bool complete_redraw_needed = false;

	// empty queues
	while (!m_chat_interface->outgoing_queue.empty()) {
		ChatEvent evt = m_chat_interface->outgoing_queue.pop_frontNoEx();
		switch (evt.type) {
			case CET_NICK_REMOVE:
				m_nicks.remove(evt.nick);
				break;
			case CET_NICK_ADD:
				m_nicks.push_back(evt.nick);
				break;
			case CET_CHAT:
				complete_redraw_needed = true;
				// This is only used for direct replies from commands
				m_chat_backend.addMessage(L"", evt.chat_msg);
				break;
		};
	}
	while (!m_log_output->queue.empty()) {
		complete_redraw_needed = true;
		std::pair<LogMessageLevel, std::string> p = m_log_output->queue.pop_frontNoEx();
		// TODO store all messages in the chat backend, regardless of logleve,
		// and only update the view
		if (p.first > m_log_level)
			continue;
		// TODO do something better with the loglevel than just itos
		m_chat_backend.addMessage(utf8_to_wide(itos(p.first)), utf8_to_wide(p.second));
	}

	// handle input
	if (!m_esc_mode) {
		handleInput(ch, complete_redraw_needed);
	} else {
		switch (ch) {
			case ERR: // no input
				break;
			case 27: // ESC
				// Toggle ESC mode
				m_esc_mode = !m_esc_mode;
				break;
			case 'L':
				m_log_level++;
				m_log_level = MYMIN(m_log_level, LMT_NUM_VALUES - 1);
				//complete_redraw_needed = true;
				break;
			case 'l':
				m_log_level--;
				m_log_level = MYMAX(m_log_level, 0);
				//complete_redraw_needed = true;
				break;
		}
	}

	// was there a resize?
	int xn, yn;
	getmaxyx(stdscr, yn, xn);
	if (xn != m_cols || yn != m_rows) {
		m_cols = xn;
		m_rows = yn;
		reformat_backend(&m_chat_backend, m_rows, m_cols);
		complete_redraw_needed = true;
	}

	// draw title
	move(0, 0);
	clrtoeol();
	printw(PROJECT_NAME_C);
	printw(" ");
	printw(g_version_hash);

	// draw text
	if (complete_redraw_needed)
		draw_text();

	// draw prompt
	if (!m_esc_mode) {
		// normal prompt
		ChatPrompt& prompt = m_chat_backend.getPrompt();
		std::string prompt_text = wide_to_utf8(prompt.getVisiblePortion());
		move(m_rows - 1, 0);
		clrtoeol();
		printw(prompt_text.c_str());
		// Draw cursor
		// TODO either here or in getVisibleCursorPosition: take care of variable length rendered characters
		s32 cursor_pos = prompt.getVisibleCursorPosition();
		if (cursor_pos >= 0) {
			move(m_rows - 1, cursor_pos);
		}
	} else {
		// esc prompt
		move(m_rows - 1, 0);
		clrtoeol();
		printw("[ESC] Toogle ESC mode | [CTRL+C] Shut down | (L) in-, (l) decrease loglevel %d", m_log_level);
	}

	refresh();
}

void TerminalChatConsole::draw_text()
{
	ChatBuffer& buf = m_chat_backend.getConsoleBuffer();
	for (u32 row = 0; row < buf.getRows(); row++) {
		move_for_backend(row, 0);
		clrtoeol();
		const ChatFormattedLine& line = buf.getFormattedLine(row);
		if (line.fragments.empty())
			continue;
		for (u32 i = 0; i < line.fragments.size(); ++i) {
			const ChatFormattedFragment& fragment = line.fragments[i];
			printw(wide_to_utf8(fragment.text).c_str());
		}
	}
}

#endif
