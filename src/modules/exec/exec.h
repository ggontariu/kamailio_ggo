/*
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/*!
 * \file
 * \brief Exec module:: Module interface
 * \ingroup exec
 * Module: \ref exec
 */

#ifndef _EXEC_H
#define _EXEC_H

#include "../../core/pvar.h"

int exec_str(struct sip_msg *msg, char *cmd, char *param, int param_len);
int exec_msg(struct sip_msg *msg, char *cmd);
int exec_avp(struct sip_msg *msg, char *cmd, pvname_list_p avpl);
int exec_cmd(sip_msg_t *msg, char *cmd);

#endif
