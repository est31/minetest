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

#ifndef TERMINAL_CHAT_CONSOLE_H
#define TERMINAL_CHAT_CONSOLE_H

#include "chat.h"
#include "threading/thread.h"
#include "chat_interface.h"
#include "log.h"

class TermLogOutput : public ILogOutput {
	// TODO: extend the interface to also send the thread name
	/* line: Only actual printed text */
	virtual void printLog(enum LogMessageLevel lev, const std::string &line)
	{
		queue.push_back(std::make_pair(lev, line));
	};

public:
	MutexedQueue<std::pair<LogMessageLevel, std::string> > queue;
};

class TerminalChatConsole : public Thread {
public:

	TerminalChatConsole(
			ChatInterface *iface,
			TermLogOutput *oput,
			bool &kill_requested,
			const std::string &nick) :
		Thread("TerminalThread"),
		m_log_level(1),
		m_nick(nick),
		m_utf8_bytes_to_wait(0),
		m_kill_requested(kill_requested),
		m_chat_interface(iface),
		m_log_output(oput),
		m_esc_mode(false)
	{
		init();
		start();
	}

	~TerminalChatConsole() { deinit(); }
	virtual void *run();

private:
	void init();
	void deinit();

	void draw_text();
	void resize();

	void typeChatMessage(const std::wstring &m);

	void handleInput(int ch, bool &complete_redraw_needed);

	void step(int ch);

	int m_log_level;
	std::string m_nick;

	u8 m_utf8_bytes_to_wait;
	std::string m_pending_utf8_bytes;

	std::list<std::string> m_nicks;
	int m_cols, m_rows;
	bool &m_kill_requested;
	ChatBackend m_chat_backend;
	ChatInterface *m_chat_interface;
	TermLogOutput *m_log_output;

	bool m_esc_mode;
};

#endif
