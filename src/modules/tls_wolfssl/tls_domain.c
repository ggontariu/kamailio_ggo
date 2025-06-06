/*
 * TLS module
 *
 * Copyright (C) 2005,2006 iptelorg GmbH
 * Copyright (C) 2013 Motorola Solutions, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * Kamailio TLS support :: Virtual domain configuration support
 * @file
 * @ingroup tls
 * Module: @ref tls
 */

#include <stdlib.h>

#include "../../core/ut.h"
#include "../../core/mem/shm_mem.h"
#include "../../core/pt.h"
#include "../../core/cfg/cfg.h"
#include "../../core/dprint.h"
#include "tls_config.h"
#include "tls_server.h"
#include "tls_util.h"
#include "tls_wolfssl_mod.h"
#include "tls_init.h"
#include "tls_domain.h"
#include "tls_cfg.h"
#include "tls_verify.h"

/*
 * needed for wolfSSL
 */
static unsigned char dh3072_p[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62,
		0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC,
		0x74, 0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08,
		0x79, 0x8E, 0x34, 0x04, 0xDD, 0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43,
		0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35,
		0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E,
		0xC6, 0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C,
		0xB6, 0xF4, 0x06, 0xB7, 0xED, 0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F,
		0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66,
		0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF,
		0x05, 0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F,
		0xA8, 0xFD, 0x24, 0xCF, 0x5F, 0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD,
		0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29,
		0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98,
		0x04, 0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E,
		0x46, 0x2E, 0x36, 0xCE, 0x3B, 0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86,
		0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F, 0xB5, 0xC5, 0x5D,
		0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17,
		0x18, 0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26,
		0x18, 0x98, 0xFA, 0x05, 0x10, 0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4,
		0x2D, 0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33, 0xA8, 0x55, 0x21,
		0xAB, 0xDF, 0x1C, 0xBA, 0x64, 0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF,
		0x0A, 0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D, 0xB3, 0x97, 0x0F,
		0x85, 0xA6, 0xE1, 0xE4, 0xC7, 0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33,
		0xD7, 0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D, 0xCE, 0xE3, 0xD2,
		0x26, 0x1A, 0xD2, 0xEE, 0x6B, 0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08,
		0x64, 0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64, 0x52, 0x1F, 0x2B,
		0x18, 0x17, 0x7B, 0x20, 0x0C, 0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D,
		0x6C, 0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2, 0x08, 0xE2, 0x4F,
		0xA0, 0x74, 0xE5, 0xAB, 0x31, 0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10,
		0x8E, 0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x3A, 0xD2, 0xCA, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF

};

static unsigned char dh3072_g[] = {0x02};

static void setup_dh(WOLFSSL_CTX *ctx)
{
	/*
 * not needed for OpenSSL 1.1.0+ and LibreSSL
 * DH_new() is deprecated in OpenSSL 3
 */
	DH *dh;
	BIGNUM *p;
	BIGNUM *g;

	dh = DH_new();
	if(dh == NULL) {
		return;
	}

	p = BN_bin2bn(dh3072_p, sizeof(dh3072_p), NULL);
	g = BN_bin2bn(dh3072_g, sizeof(dh3072_g), NULL);

	if(p == NULL || g == NULL) {
		DH_free(dh);
		return;
	}

	dh->p = p;
	dh->g = g;


	wolfSSL_CTX_set_options(ctx, WOLFSSL_OP_SINGLE_DH_USE);
	wolfSSL_CTX_set_tmp_dh(ctx, dh);

	DH_free(dh);
}


/**
 * @brief Create a new TLS domain structure
 *
 * Create a new domain structure in new allocated shared memory.
 * @param type domain Type
 * @param ip domain IP
 * @param port domain port
 * @return new domain
 */
tls_domain_t *tls_new_domain(int type, struct ip_addr *ip, unsigned short port)
{
	tls_domain_t *d;

	d = shm_malloc(sizeof(tls_domain_t));
	if(d == NULL) {
		ERR("Memory allocation failure\n");
		return 0;
	}
	memset(d, '\0', sizeof(tls_domain_t));

	d->type = type;
	if(ip)
		memcpy(&d->ip, ip, sizeof(struct ip_addr));
	d->port = port;
	d->verify_cert = -1;
	d->verify_depth = -1;
	d->require_cert = -1;
	d->verify_client = -1;
	return d;
}


/**
 * @brief Free all memory used by TLS configuration domain
 * @param d freed domain
 */
void tls_free_domain(tls_domain_t *d)
{
	if(!d)
		return;
	if(d->ctx) {
		do {
			if(d->ctx[0])
				wolfSSL_CTX_free(d->ctx[0]);
		} while(0);
		shm_free(d->ctx);
	}

	if(d->cipher_list.s)
		shm_free(d->cipher_list.s);
	if(d->ca_file.s)
		shm_free(d->ca_file.s);
	if(d->ca_path.s)
		shm_free(d->ca_path.s);
	if(d->crl_file.s)
		shm_free(d->crl_file.s);
	if(d->pkey_file.s)
		shm_free(d->pkey_file.s);
	if(d->cert_file.s)
		shm_free(d->cert_file.s);
	if(d->server_name.s)
		shm_free(d->server_name.s);
	if(d->server_id.s)
		shm_free(d->server_id.s);
	shm_free(d);
}


/**
 * @brief Free TLS configuration structure
 * @param cfg freed configuration
 */
void tls_free_cfg(tls_domains_cfg_t *cfg)
{
	tls_domain_t *p;
	while(cfg->srv_list) {
		p = cfg->srv_list;
		cfg->srv_list = cfg->srv_list->next;
		tls_free_domain(p);
	}
	while(cfg->cli_list) {
		p = cfg->cli_list;
		cfg->cli_list = cfg->cli_list->next;
		tls_free_domain(p);
	}
	if(cfg->srv_default)
		tls_free_domain(cfg->srv_default);
	if(cfg->cli_default)
		tls_free_domain(cfg->cli_default);
	shm_free(cfg);
}


/**
 * @brief Destroy all TLS configuration data
 */
void tls_destroy_cfg(void)
{
	tls_domains_cfg_t *ptr;

	if(tls_domains_cfg_lock) {
		lock_destroy(tls_domains_cfg_lock);
		lock_dealloc(tls_domains_cfg_lock);
		tls_domains_cfg_lock = 0;
	}

	if(tls_domains_cfg) {
		while(*tls_domains_cfg) {
			ptr = *tls_domains_cfg;
			*tls_domains_cfg = (*tls_domains_cfg)->next;
			tls_free_cfg(ptr);
		}

		shm_free(tls_domains_cfg);
		tls_domains_cfg = 0;
	}
}


/**
 * @brief Generate TLS domain identifier
 * @param d printed domain
 * @return printed domain, with zero termination
 */
char *tls_domain_str(tls_domain_t *d)
{
	static char buf[1024];
	char *p;

	buf[0] = '\0';
	p = buf;
	p = strcat(p, d->type & TLS_DOMAIN_SRV ? "TLSs<" : "TLSc<");
	if(d->type & TLS_DOMAIN_DEF) {
		p = strcat(p, "default>");
	} else if(d->type & TLS_DOMAIN_ANY) {
		p = strcat(p, "any:");
		if(d->server_name.s && d->server_name.len > 0) {
			p = strncat(p, d->server_name.s, d->server_name.len);
		}
		p = strcat(p, ">");
	} else {
		p = strcat(p, ip_addr2a(&d->ip));
		p = strcat(p, ":");
		p = strcat(p, int2str(d->port, 0));
		p = strcat(p, ">");
	}
	return buf;
}


/**
 * @brief Initialize TLS domain parameters that have not been configured yet
 *
 * Initialize TLS domain parameters that have not been configured from
 * parent domain (usually one of default domains)
 * @param d initialized domain
 * @param parent parent domain
 * @return 0 on success, -1 on error
 */
static int ksr_tls_fill_missing(tls_domain_t *d, tls_domain_t *parent)
{
	if(d->method == TLS_METHOD_UNSPEC)
		d->method = parent->method;
	LOG(L_INFO, "%s: tls_method=%d\n", tls_domain_str(d), d->method);

	if(d->method < 1 || d->method >= TLS_METHOD_MAX) {
		ERR("%s: Invalid TLS method value\n", tls_domain_str(d));
		return -1;
	}

	if(!d->cert_file.s) {
		if(shm_asciiz_dup(&d->cert_file.s, parent->cert_file.s) < 0)
			return -1;
		d->cert_file.len = parent->cert_file.len;
	}
	LOG(L_INFO, "%s: certificate='%s'\n", tls_domain_str(d), d->cert_file.s);

	if(!d->ca_file.s) {
		if(shm_asciiz_dup(&d->ca_file.s, parent->ca_file.s) < 0)
			return -1;
		d->ca_file.len = parent->ca_file.len;
	}
	LOG(L_INFO, "%s: ca_list='%s'\n", tls_domain_str(d), d->ca_file.s);

	if(!d->ca_path.s) {
		if(shm_asciiz_dup(&d->ca_path.s, parent->ca_path.s) < 0)
			return -1;
		d->ca_path.len = parent->ca_path.len;
	}
	LOG(L_INFO, "%s: ca_path='%s'\n", tls_domain_str(d), d->ca_path.s);

	if(!d->crl_file.s) {
		if(shm_asciiz_dup(&d->crl_file.s, parent->crl_file.s) < 0)
			return -1;
		d->crl_file.len = parent->crl_file.len;
	}
	LOG(L_INFO, "%s: crl='%s'\n", tls_domain_str(d), d->crl_file.s);

	if(d->require_cert == -1)
		d->require_cert = parent->require_cert;
	LOG(L_INFO, "%s: require_certificate=%d\n", tls_domain_str(d),
			d->require_cert);

	if(!d->cipher_list.s) {
		if(shm_asciiz_dup(&d->cipher_list.s, parent->cipher_list.s) < 0)
			return -1;
		d->cipher_list.len = parent->cipher_list.len;
	}
	LOG(L_INFO, "%s: cipher_list='%s'\n", tls_domain_str(d), d->cipher_list.s);

	if(!d->pkey_file.s) {
		if(shm_asciiz_dup(&d->pkey_file.s, parent->pkey_file.s) < 0)
			return -1;
		d->pkey_file.len = parent->pkey_file.len;
	}
	LOG(L_INFO, "%s: private_key='%s'\n", tls_domain_str(d), d->pkey_file.s);

	if(d->verify_cert == -1)
		d->verify_cert = parent->verify_cert;
	LOG(L_INFO, "%s: verify_certificate=%d\n", tls_domain_str(d),
			d->verify_cert);

	if(d->verify_depth == -1)
		d->verify_depth = parent->verify_depth;
	LOG(L_INFO, "%s: verify_depth=%d\n", tls_domain_str(d), d->verify_depth);

	if(d->verify_client == -1)
		d->verify_client = parent->verify_client;
	LOG(L_INFO, "%s: verify_client=%d\n", tls_domain_str(d), d->verify_client);

	return 0;
}


/**
 * @brief Called for ctx, with 2 args
 * @param ctx SSL context
 * @param larg ?
 * @param parg ?
 * @return return 0 on success, <0 on critical error
 */
typedef int (*per_ctx_cbk_f)(WOLFSSL_CTX *ctx, long larg, void *parg);


/**
 * @brief Execute callback on all the CTX'es on a domain
 * @param d domain
 * @param ctx_cbk callback function
 * @param l1 parameter passed to the callback
 * @param p2 parameter passed to the callback
 * @return 0 on success, <0 on error
 */
static int tls_domain_foreach_CTX(
		tls_domain_t *d, per_ctx_cbk_f ctx_cbk, long l1, void *p2)
{
	int ret;

	do {
		if((ret = ctx_cbk(d->ctx[0], l1, p2)) < 0)
			return ret;
	} while(0);
	return 0;
}


/**
 * @brief Execute callback on all the CTX'es on in a domain list
 * @param d domain
 * @param ctx_cbk callback function
 * @param l1 parameter passed to the callback
 * @param p2 parameter passed to the callback
 * @return 0 on success, <0 on error
 */
static int tls_foreach_CTX_in_domain_lst(
		tls_domain_t *d, per_ctx_cbk_f ctx_cbk, long l1, void *p2)
{
	int ret;
	for(; d; d = d->next)
		if((ret = tls_domain_foreach_CTX(d, ctx_cbk, l1, p2)) < 0)
			return ret;
	return 0;
}


/**
 * @brief Execute callback on all the CTX'es in all the srv domains in a tls cfg
 * @param cfg tls cfg.
 * @param ctx_cbk callback function
 * @param l1 parameter passed to the callback
 * @param p2 parameter passed to the callback
 * @return 0 on success, <0 on error
 */
static int tls_foreach_CTX_in_srv_domains(
		tls_domains_cfg_t *cfg, per_ctx_cbk_f ctx_cbk, long l1, void *p2)
{
	int ret;
	if((ret = tls_domain_foreach_CTX(cfg->srv_default, ctx_cbk, l1, p2)) < 0)
		return ret;
	if((ret = tls_foreach_CTX_in_domain_lst(cfg->srv_list, ctx_cbk, l1, p2))
			< 0)
		return ret;
	return 0;
}


/**
 * @brief Execute callback on all the CTX'es in all the client domains in a tls cfg
 * @param cfg tls cfg.
 * @param ctx_cbk callback function
 * @param l1 parameter passed to the callback
 * @param p2 parameter passed to the callback
 * @return 0 on success, <0 on error.
 */
static int tls_foreach_CTX_in_cli_domains(
		tls_domains_cfg_t *cfg, per_ctx_cbk_f ctx_cbk, long l1, void *p2)
{
	int ret;
	if((ret = tls_domain_foreach_CTX(cfg->cli_default, ctx_cbk, l1, p2)) < 0)
		return ret;
	if((ret = tls_foreach_CTX_in_domain_lst(cfg->cli_list, ctx_cbk, l1, p2))
			< 0)
		return ret;
	return 0;
}


/**
 * @brief Execute callback on all the CTX'es in all the domains in a tls cfg
 * @param cfg tls cfg
 * @param ctx_cbk callback function
 * @param l1 parameter passed to the callback
 * @param p2 parameter passed to the callback
 * @return 0 on success, <0 on error
 */
static int tls_foreach_CTX_in_cfg(
		tls_domains_cfg_t *cfg, per_ctx_cbk_f ctx_cbk, long l1, void *p2)
{
	int ret;

	if((ret = tls_foreach_CTX_in_srv_domains(cfg, ctx_cbk, l1, p2)) < 0)
		return ret;
	if((ret = tls_foreach_CTX_in_cli_domains(cfg, ctx_cbk, l1, p2)) < 0)
		return ret;
	return 0;
}


/**
 * @brief Fix pathnames when loading domain keys or other list
 *
 * Fix pathnames, to be used when loading the domain key, cert, ca list a.s.o.
 * It will replace path with a fixed shm allocated version. Assumes path->s
 * was shm allocated.
 * @param path path to be fixed. If it starts with '.' or '/' is left alone
 * (forced "relative" or "absolute" path). Otherwise the path is considered
 * to be relative to the main config file directory
 * (e.g. for /etc/ser/ser.cfg => /etc/ser/\<path\>).
 * @return  0 on success, -1 on error
 */
int fix_shm_pathname(str *path)
{
	str new_path;
	char *abs_path;

	if(path->s && path->len && *path->s != '.' && *path->s != '/') {
		abs_path = get_abs_pathname(0, path);
		if(abs_path == 0) {
			LM_ERR("get abs pathname failed\n");
			return -1;
		}
		new_path.len = strlen(abs_path);
		new_path.s = shm_malloc(new_path.len + 1);
		if(new_path.s == 0) {
			LM_ERR("no more shm memory\n");
			pkg_free(abs_path);
			return -1;
		}
		memcpy(new_path.s, abs_path, new_path.len);
		new_path.s[new_path.len] = 0;
		shm_free(path->s);
		pkg_free(abs_path);
		*path = new_path;
	}
	return 0;
}


/**
 * @brief Load certificate from file
 * @param d domain
 * @return 0 if not configured or on success, -1 on error
 */
static int load_cert(tls_domain_t *d)
{
	if(!d->cert_file.s || !d->cert_file.len) {
		DBG("%s: No certificate configured\n", tls_domain_str(d));
		return 0;
	}
	if(fix_shm_pathname(&d->cert_file) < 0)
		return -1;

	do {
		if(!wolfSSL_CTX_use_certificate_chain_file(d->ctx[0], d->cert_file.s)) {
			ERR("%s: Unable to load certificate file '%s'\n", tls_domain_str(d),
					d->cert_file.s);
			TLS_ERR("load_cert:");
			return -1;
		}
	} while(0);
	return 0;
}


/**
 * @brief Load CA list from file
 * @param d domain
 * @return 0 if not configured or on success, -1 on error
 */
static int load_ca_list(tls_domain_t *d)
{
	if((!d->ca_file.s || !d->ca_file.len)
			&& (!d->ca_path.s || !d->ca_path.len)) {
		DBG("%s: No CA list configured\n", tls_domain_str(d));
		return 0;
	}
	if(d->ca_file.s && d->ca_file.len > 0 && fix_shm_pathname(&d->ca_file) < 0)
		return -1;
	if(d->ca_path.s && d->ca_path.len > 0 && fix_shm_pathname(&d->ca_path) < 0)
		return -1;

	do {
		if(wolfSSL_CTX_load_verify_locations(
				   d->ctx[0], d->ca_file.s, d->ca_path.s)
				!= 1) {
			ERR("%s: Unable to load CA list file '%s' dir '%s'\n",
					tls_domain_str(d), (d->ca_file.s) ? d->ca_file.s : "",
					(d->ca_path.s) ? d->ca_path.s : "");
			TLS_ERR("load_ca_list:");
			return -1;
		}
		if(d->ca_file.s && d->ca_file.len > 0) {
			wolfSSL_CTX_set_client_CA_list(
					d->ctx[0], wolfSSL_load_client_CA_file(d->ca_file.s));
			if(wolfSSL_CTX_get_client_CA_list(d->ctx[0]) == 0) {
				ERR("%s: Error while setting client CA list file [%.*s/%d]\n",
						tls_domain_str(d), (d->ca_file.s) ? d->ca_file.len : 0,
						(d->ca_file.s) ? d->ca_file.s : "", d->ca_file.len);
				TLS_ERR("load_ca_list:");
				return -1;
			}
		}
	} while(0);
	return 0;
}


/**
 * @brief Load CRL from file
 * @param d domain
 * @return 0 if not configured or on success, -1 on error
 */
static int load_crl(tls_domain_t *d)
{
	WOLFSSL_X509_STORE *store;

	if(!d->crl_file.s) {
		DBG("%s: No CRL configured\n", tls_domain_str(d));
		return 0;
	}
	if(fix_shm_pathname(&d->crl_file) < 0)
		return -1;
	LOG(L_INFO, "%s: Certificate revocation lists will be checked (%.*s)\n",
			tls_domain_str(d), d->crl_file.len, d->crl_file.s);

	do {
		if(wolfSSL_CTX_load_verify_locations(d->ctx[0], d->crl_file.s, 0)
				!= 1) {
			ERR("%s: Unable to load certificate revocation list '%s'\n",
					tls_domain_str(d), d->crl_file.s);
			TLS_ERR("load_crl:");
			return -1;
		}
		store = wolfSSL_CTX_get_cert_store(d->ctx[0]);
		wolfSSL_X509_STORE_set_flags(
				store, WOLFSSL_CRL_CHECK | WOLFSSL_CRL_CHECKALL);
	} while(0);
	return 0;
}


#define C_DEF_NO_KRB5 "DEFAULT:!KRB5"
#define C_DEF_NO_KRB5_LEN (sizeof(C_DEF_NO_KRB5) - 1)
#define C_NO_KRB5_SUFFIX ":!KRB5"
#define C_NO_KRB5_SUFFIX_LEN (sizeof(C_NO_KRB5_SUFFIX) - 1)

/**
 * @brief Configure cipher list
 * @param d domain
 * @return 0 on success, -1 on error
 */
static int set_cipher_list(tls_domain_t *) __attribute__((unused));
static int set_cipher_list(tls_domain_t *d)
{
	char *cipher_list;

	cipher_list = d->cipher_list.s;
	if(!cipher_list)
		return 0;

	do {
		if(wolfSSL_CTX_set_cipher_list(d->ctx[0], cipher_list) == 0) {
			ERR("%s: Failure to set SSL context cipher list \"%s\"\n",
					tls_domain_str(d), cipher_list);
			return -1;
		}
		setup_dh(d->ctx[0]);
	} while(0);
	return 0;
}


/**
 * @brief Enable/disable TLS certificate verification
 * @param d domain
 * @return 0
 */
static int set_verification(tls_domain_t *d)
{
	int verify_mode;

	if(d->require_cert || d->verify_client == TLS_VERIFY_CLIENT_ON) {
		verify_mode = WOLFSSL_VERIFY_PEER | WOLFSSL_VERIFY_FAIL_IF_NO_PEER_CERT;
		LOG(L_INFO, "%s: %s MUST present valid certificate\n",
				tls_domain_str(d),
				d->type & TLS_DOMAIN_SRV ? "Client" : "Server");
	} else {
		if(d->verify_cert || d->verify_client >= TLS_VERIFY_CLIENT_OPTIONAL) {
			verify_mode = WOLFSSL_VERIFY_PEER;
			if(d->type & TLS_DOMAIN_SRV) {
				LOG(L_INFO,
						"%s: IF client provides certificate then it"
						" MUST be valid\n",
						tls_domain_str(d));
			} else {
				LOG(L_INFO, "%s: Server MUST present valid certificate\n",
						tls_domain_str(d));
			}
		} else {
			verify_mode = WOLFSSL_VERIFY_NONE;
			if(d->type & TLS_DOMAIN_SRV) {
				LOG(L_INFO,
						"%s: No client certificate required and no checks"
						" performed\n",
						tls_domain_str(d));
			} else {
				LOG(L_INFO, "%s: Server MAY present invalid certificate\n",
						tls_domain_str(d));
			}
		}
	}

	do {
		if(d->verify_client >= TLS_VERIFY_CLIENT_OPTIONAL_NO_CA) {
			/* Note that actual verification result is available in $tls_peer_verified */
			wolfSSL_CTX_set_verify(d->ctx[0], verify_mode,
					verify_callback_unconditional_success);
		} else {
			wolfSSL_CTX_set_verify(d->ctx[0], verify_mode, 0);
		}
		wolfSSL_CTX_set_verify_depth(d->ctx[0], d->verify_depth);
	} while(0);
	return 0;
}


/* This callback function is executed when libssl processes the SSL
 * handshake and does SSL record layer stuff. It's used to trap
 * client-initiated renegotiations.
 */

static void sr_ssl_ctx_info_callback(const SSL *ssl, int event, int ret)
{
	struct tls_extra_data *data = 0;
	int tls_dbg;

	if(event & SSL_CB_HANDSHAKE_START) {
		tls_dbg = cfg_get(tls, tls_cfg, debug);
		LOG(tls_dbg, "SSL handshake started\n");
		if(data == 0)
			data = (struct tls_extra_data *)wolfSSL_get_app_data(ssl);
		if(data->flags & F_TLS_CON_HANDSHAKED) {
			LOG(tls_dbg, "SSL renegotiation initiated by client\n");
			data->flags |= F_TLS_CON_RENEGOTIATION;
		}
	}
	if(event & SSL_CB_HANDSHAKE_DONE) {
		tls_dbg = cfg_get(tls, tls_cfg, debug);
		if(data == 0)
			data = (struct tls_extra_data *)wolfSSL_get_app_data(ssl);
		LOG(tls_dbg, "SSL handshake done\n");
		data->flags |= F_TLS_CON_HANDSHAKED;
	}
}

/**
 * @brief Configure generic SSL parameters
 * @param d domain
 * @return 0
 */
static int set_ssl_options(tls_domain_t *d)
{
	long options;

	options = SSL_OP_ALL; /* all the bug workarounds by default */
	options |= SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
			   | WOLFSSL_OP_CIPHER_SERVER_PREFERENCE;

	do {
		wolfSSL_CTX_set_options(d->ctx[0], options);
		if(sr_tls_renegotiation == 0)
			wolfSSL_CTX_set_info_callback(d->ctx[0], sr_ssl_ctx_info_callback);
	} while(0);
	return 0;
}


/**
 * @brief Configure TLS session cache parameters
 * @param d domain
 * @return 0
 */
static int set_session_cache(tls_domain_t *d)
{
	str tls_session_id;

	tls_session_id = cfg_get(tls, tls_cfg, session_id);
	do {
		/* janakj: I am not sure if session cache makes sense in ser, session
		 * cache is stored in SSL_CTX and we have one SSL_CTX per process,
		 * thus sessions among processes will not be reused
		 */
		wolfSSL_CTX_set_session_cache_mode(d->ctx[0],
				cfg_get(tls, tls_cfg, session_cache) ? WOLFSSL_SESS_CACHE_SERVER
													 : WOLFSSL_SESS_CACHE_OFF);
		/* not really needed is SSL_SESS_CACHE_OFF */
		wolfSSL_CTX_set_session_id_context(d->ctx[0],
				(unsigned char *)tls_session_id.s, tls_session_id.len);
	} while(0);
	return 0;
}


/**
 * @brief TLS SSL_CTX_set_mode and SSL_CTX_clear_mode wrapper
 * @param ctx SSL context
 * @param mode WOLFSSL_MODE_*
 * @param clear if set to !=0 will do a clear, else (==0) a set
 * @return 0 (always succeeds)
 */
static int tls_ssl_ctx_mode(WOLFSSL_CTX *ctx, long mode, void *clear)
{
	if(clear)
		wolfSSL_CTX_clear_mode(ctx, mode);

	else
		wolfSSL_CTX_set_mode(ctx, mode);
	return 0;
}


/**
 * @brief TLS set ctx->free_list_max_len
 * @param ctx TLS context
 * @param val value (<0 ignored)
 * @param unused unused
 * @return 0 (always succeeds)
 */
static int tls_ssl_ctx_set_freelist(WOLFSSL_CTX *ctx, long val, void *unused)
{
	/* NOOP */
	return 0;
}

/**
 * @brief TLS SSL_CTX_set_max_send_fragment wrapper
 * @param ctx TLS context
 * @param val value (<0 ignored). Should be between 512 and 16k
 * @param unused unused
 * @return 0 on success, < 0 on failure (invalid value)
 */
static int tls_ssl_ctx_set_max_send_fragment(
		WOLFSSL_CTX *ctx, long val, void *unused)
{
	/* NOOP */
	return 0;
}


/**
 * @brief TLS SSL_CTX_set_read_ahead wrapper
 * @param ctx TLS context
 * @param val value (<0 ignored, 0 or >0)
 * @param unused unused
 * @return 0 (always success).
 */
static int tls_ssl_ctx_set_read_ahead(WOLFSSL_CTX *ctx, long val, void *unused)
{
	if(val >= 0)
		wolfSSL_CTX_set_read_ahead(ctx, val);
	return 0;
}


/**
 * @brief SNI callback function
 *
 * callback on server_name -> trigger context switch if a TLS domain
 * for the server_name is found (checks socket too) */
static int tls_server_name_cb(SSL *ssl, int *ad, void *private)
{
	tls_domain_t *orig_domain, *new_domain;
	str server_name;

	orig_domain = (tls_domain_t *)private;
	server_name.s = (char *)wolfSSL_get_servername(ssl, WOLFSSL_SNI_HOST_NAME);
	if(server_name.s) {
		LM_DBG("received server_name (TLS extension): '%s'\n", server_name.s);
	} else {
		LM_DBG("SSL_get_servername returned NULL: return "
			   "SSL_TLSEXT_ERR_NOACK\n");
		return SSL_TLSEXT_ERR_NOACK;
	}

	server_name.len = strlen(server_name.s);

	new_domain = tls_lookup_cfg(*tls_domains_cfg, TLS_DOMAIN_SRV,
			&orig_domain->ip, orig_domain->port, &server_name, 0);
	if(new_domain == NULL) {
		LM_DBG("TLS domain for socket [%s:%d] and server_name='%s' "
			   "not found\n",
				ip_addr2a(&orig_domain->ip), orig_domain->port, server_name.s);
		/* we do not perform SSL_CTX switching, thus the default server domain
		   for this socket (or the default server domain) will be used. */
		return SSL_TLSEXT_ERR_ALERT_WARNING;
	}

	LM_DBG("TLS cfg domain selected for received server name [%s]:"
		   " socket [%s:%d] server name='%s' -"
		   " switching SSL CTX to %p dom %p%s\n",
			server_name.s, ip_addr2a(&new_domain->ip), new_domain->port,
			ZSW(new_domain->server_name.s), new_domain->ctx[0], new_domain,
			(new_domain->type & TLS_DOMAIN_DEF) ? " (default)" : "");
	wolfSSL_set_SSL_CTX(ssl, new_domain->ctx[0]);
	/* SSL_set_SSL_CTX only sets the correct certificate parameters, but does
	   set the proper verify options. Thus this will be done manually! */

	wolfSSL_set_options(ssl, wolfSSL_CTX_get_options(wolfSSL_get_SSL_CTX(ssl)));
	if((wolfSSL_get_verify_mode(ssl) == WOLFSSL_VERIFY_NONE)
			|| (wolfSSL_num_renegotiations(ssl) == 0)) {
		/*
		 * Only initialize the verification settings from the ctx
		 * if they are not yet set, or if we're called when a new
		 * SSL connection is set up (num_renegotiations == 0).
		 * Otherwise, we would possibly reset a per-directory
		 * configuration which was put into effect by ssl_hook_access.
		 */
		wolfSSL_set_verify(ssl,
				wolfSSL_CTX_get_verify_mode(wolfSSL_get_SSL_CTX(ssl)),
				wolfSSL_CTX_get_verify_callback(wolfSSL_get_SSL_CTX(ssl)));
	}

	LM_DBG("tls_server_name_cb return SSL_TLSEXT_ERR_OK");
	return SSL_TLSEXT_ERR_OK;
}


/**
 * @brief Initialize all domain attributes from default domains if necessary
 * @param d initialized TLS domain
 * @param def default TLS domains
 */
static int ksr_tls_fix_domain(tls_domain_t *d, tls_domain_t *def)
{
	if(ksr_tls_fill_missing(d, def) < 0)
		return -1;

	if(d->type & TLS_DOMAIN_ANY) {
		if(d->server_name.s == NULL || d->server_name.len < 0) {
			LM_ERR("%s: tls domain for any address but no server name\n",
					tls_domain_str(d));
			return -1;
		}
	}

	d->ctx = (WOLFSSL_CTX **)shm_malloc(sizeof(WOLFSSL_CTX *) * 1);
	if(!d->ctx) {
		ERR("%s: Cannot allocate shared memory\n", tls_domain_str(d));
		return -1;
	}
	if(d->method > TLS_USE_TLSvRANGE) {
		LM_DBG("using tls methods range: %d\n", d->method);
	} else {
		LM_DBG("using one tls method version: %d\n", d->method);
	}
	memset(d->ctx, 0, sizeof(WOLFSSL_CTX *) * 1);
	do {

		/* libssl >= 1.1.0 */
		//d->ctx[i] = SSL_CTX_new(sr_tls_methods[d->method - 1].TLSMethod);
		d->ctx[0] = wolfSSL_CTX_new(wolfSSLv23_method());
		if(d->ctx[0] == NULL) {
			unsigned long e = 0;
			e = ERR_peek_last_error();
			ERR("%s: Cannot create SSL context [%d] (%lu: %s / %s)\n",
					tls_domain_str(d), 0, e, ERR_error_string(e, NULL),
					ERR_reason_error_string(e));
			return -1;
		}
		wolfSSL_CTX_set_min_proto_version(d->ctx[0], TLS1_2_VERSION);

		/*
		* check server domains for server_name extension and register
		* callback function
		*/
		if((d->type & TLS_DOMAIN_SRV)
				&& (d->server_name.len > 0 || (d->type & TLS_DOMAIN_DEF))) {
			if(!wolfSSL_CTX_set_tlsext_servername_callback(
					   d->ctx[0], tls_server_name_cb)) {
				LM_ERR("register server_name callback handler for socket "
					   "[%s:%d], server_name='%s' failed for proc %d\n",
						ip_addr2a(&d->ip), d->port,
						(d->server_name.s) ? d->server_name.s : "<default>", 0);
				return -1;
			}
			if(!wolfSSL_CTX_set_servername_arg(d->ctx[0], d)) {
				LM_ERR("register server_name callback handler data for socket "
					   "[%s:%d], server_name='%s' failed for proc %d\n",
						ip_addr2a(&d->ip), d->port,
						(d->server_name.s) ? d->server_name.s : "<default>", 0);
				return -1;
			}
		}
	} while(0);

	if((d->type & TLS_DOMAIN_SRV)
			&& (d->server_name.len > 0 || (d->type & TLS_DOMAIN_DEF))) {
		LM_NOTICE("registered server_name callback handler for socket "
				  "[%s:%d], server_name='%s' ...\n",
				ip_addr2a(&d->ip), d->port,
				(d->server_name.s) ? d->server_name.s : "<default>");
	}

	if(load_cert(d) < 0)
		return -1;
	if(load_ca_list(d) < 0)
		return -1;
	if(load_crl(d) < 0)
		return -1;
	if(set_cipher_list(d) < 0)
		return -1;
	if(set_verification(d) < 0)
		return -1;
	if(set_ssl_options(d) < 0)
		return -1;
	if(set_session_cache(d) < 0)
		return -1;
	return 0;
}

/**
 * @brief Load a private key from a file
 * @param d TLS domain
 * @return 0 on success, -1 on error
 */
static int load_private_key(tls_domain_t *d)
{
	int idx, ret_pwd;

	if(!d->pkey_file.s || !d->pkey_file.len) {
		DBG("%s: No private key specified\n", tls_domain_str(d));
		return 0;
	}
	if(fix_shm_pathname(&d->pkey_file) < 0)
		return -1;

	do {

		for(idx = 0, ret_pwd = 0; idx < 3; idx++) {
			ret_pwd = wolfSSL_CTX_use_PrivateKey_file(
					d->ctx[0], d->pkey_file.s, SSL_FILETYPE_PEM);
			if(ret_pwd) {
				break;
			} else {
				ERR("%s: Unable to load private key '%s'\n", tls_domain_str(d),
						d->pkey_file.s);
				TLS_ERR("load_private_key:");
				continue;
			}
		}

		if(!ret_pwd) {
			ERR("%s: Unable to load private key file '%s'\n", tls_domain_str(d),
					d->pkey_file.s);
			TLS_ERR("load_private_key:");
			return -1;
		}
		if(!wolfSSL_CTX_check_private_key(d->ctx[0])) {
			ERR("%s: Key '%s' does not match the public key of the"
				" certificate\n",
					tls_domain_str(d), d->pkey_file.s);
			TLS_ERR("load_private_key:");
			return -1;
		}
	} while(0);

	DBG("%s: Key '%s' successfully loaded\n", tls_domain_str(d),
			d->pkey_file.s);
	return 0;
}

/**
 * @brief Initialize attributes of all domains from default domains if necessary
 *
 * Initialize attributes of all domains from default domains if necessary,
 * fill in missing parameters.
 * @param cfg initialized domain
 * @param srv_defaults server defaults
 * @param cli_defaults command line interface defaults
 * @return 0 on success, -1 on error
 */
int tls_fix_domains_cfg(tls_domains_cfg_t *cfg, tls_domain_t *srv_defaults,
		tls_domain_t *cli_defaults)
{
	tls_domain_t *d;
	int ssl_mode_release_buffers;
	int ssl_freelist_max_len;
	int ssl_max_send_fragment;
	int ssl_read_ahead;

	if(!cfg->cli_default) {
		cfg->cli_default =
				tls_new_domain(TLS_DOMAIN_DEF | TLS_DOMAIN_CLI, 0, 0);
	}

	if(!cfg->srv_default) {
		cfg->srv_default =
				tls_new_domain(TLS_DOMAIN_DEF | TLS_DOMAIN_SRV, 0, 0);
	}

	if(ksr_tls_fix_domain(cfg->srv_default, srv_defaults) < 0)
		return -1;
	if(ksr_tls_fix_domain(cfg->cli_default, cli_defaults) < 0)
		return -1;

	d = cfg->srv_list;
	while(d) {
		if(ksr_tls_fix_domain(d, srv_defaults) < 0)
			return -1;
		d = d->next;
	}

	d = cfg->cli_list;
	while(d) {
		if(ksr_tls_fix_domain(d, cli_defaults) < 0)
			return -1;
		d = d->next;
	}

	/* Ask for passwords as the last step */
	d = cfg->srv_list;
	while(d) {
		if(load_private_key(d) < 0)
			return -1;
		d = d->next;
	}

	d = cfg->cli_list;
	while(d) {
		if(load_private_key(d) < 0)
			return -1;
		d = d->next;
	}

	if(load_private_key(cfg->srv_default) < 0)
		return -1;
	if(load_private_key(cfg->cli_default) < 0)
		return -1;

	/* set various global per CTX options
	 * (done here to show possible missing features messages only once)
	 */
	ssl_mode_release_buffers = cfg_get(tls, tls_cfg, ssl_release_buffers);
	ssl_freelist_max_len = cfg_get(tls, tls_cfg, ssl_freelist_max);
	ssl_max_send_fragment = cfg_get(tls, tls_cfg, ssl_max_send_fragment);
	ssl_read_ahead = cfg_get(tls, tls_cfg, ssl_read_ahead);

	/* set SSL_MODE_RELEASE_BUFFERS if ssl_mode_release_buffers !=0,
	   reset if == 0 and ignore if < 0 */
	/* only in >= 1.0.0 */
	if(ssl_mode_release_buffers >= 0
			&& tls_foreach_CTX_in_cfg(cfg, tls_ssl_ctx_mode,
					   SSL_MODE_RELEASE_BUFFERS,
					   (void *)(long)(ssl_mode_release_buffers == 0))
					   < 0) {
		ERR("invalid ssl_release_buffers value (%d)\n",
				ssl_mode_release_buffers);
		return -1;
	}
	/* only in >= 1.0.0 */

	if(tls_foreach_CTX_in_cfg(
			   cfg, tls_ssl_ctx_set_freelist, ssl_freelist_max_len, 0)
			< 0) {
		ERR("invalid ssl_freelist_max_len value (%d)\n", ssl_freelist_max_len);
		return -1;
	}

	/* only in >= 0.9.9 */
	if(tls_foreach_CTX_in_cfg(
			   cfg, tls_ssl_ctx_set_max_send_fragment, ssl_max_send_fragment, 0)
			< 0) {
		ERR("invalid ssl_max_send_fragment value (%d)\n",
				ssl_max_send_fragment);
		return -1;
	}
	if(tls_foreach_CTX_in_cfg(
			   cfg, tls_ssl_ctx_set_read_ahead, ssl_read_ahead, 0)
			< 0) {
		ERR("invalid ssl_read_ahead value (%d)\n", ssl_read_ahead);
		return -1;
	}
	/* set options for SSL_write:
		WOLFSSL_MODE_ACCEPT_MOVING_WRITE_BUFFER - needed when queueing
		  clear text for a future write (WANTS_READ). In this case the
		  buffer address will change for the repeated SSL_write() and
		  without this option it will trigger the openssl sanity checks.
		SSL_MODE_ENABLE_PARTIAL_WRITE - needed to deal with potentially
		  huge multi-record writes that don't fit in the default buffer
		  (the default buffer must have space for at least 1 record) */
	if(tls_foreach_CTX_in_cfg(cfg, tls_ssl_ctx_mode,
			   WOLFSSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
					   | SSL_MODE_ENABLE_PARTIAL_WRITE,
			   0)
			< 0) {
		ERR("could not set WOLFSSL_MODE_ACCEPT_MOVING_WRITE_BUFFER and"
			" SSL_MODE_ENABLE_PARTIAL_WRITE\n");
		return -1;
	}

	return 0;
}


/**
 * @brief Create new configuration structure
 *
 * Create new configuration structure in new allocated shared memory
 * @return configuration structure or zero on error
 */
tls_domains_cfg_t *tls_new_cfg(void)
{
	tls_domains_cfg_t *r;

	r = (tls_domains_cfg_t *)shm_malloc(sizeof(tls_domains_cfg_t));
	if(!r) {
		ERR("No memory left\n");
		return 0;
	}
	memset(r, 0, sizeof(tls_domains_cfg_t));
	atomic_set(&r->ref_count, 0);
	return r;
}


/**
 * @brief Lookup TLS configuration based on type, ip, and port
 * @param cfg configuration set
 * @param type type of configuration
 * @param ip IP for configuration
 * @param port port for configuration
 * @return found configuration or default, if not found
 */
tls_domain_t *tls_lookup_cfg(tls_domains_cfg_t *cfg, int type,
		struct ip_addr *ip, unsigned short port, str *sname, str *srvid)
{
	tls_domain_t *p;
	int dotpos;

	if(type & TLS_DOMAIN_DEF) {
		if(type & TLS_DOMAIN_SRV)
			return cfg->srv_default;
		else
			return cfg->cli_default;
	} else {
		if(type & TLS_DOMAIN_SRV)
			p = cfg->srv_list;
		else
			p = cfg->cli_list;
	}

	while(p) {
		if(srvid && srvid->len > 0) {
			LM_DBG("comparing addr: [%s:%d]  [%s:%d] -- id: [%.*s] [%.*s]\n",
					ip_addr2a(&p->ip), p->port, ip_addr2a(ip), port,
					p->server_id.len, ZSW(p->server_id.s), srvid->len,
					ZSW(srvid->s));
			if(p->server_id.s && p->server_id.len == srvid->len
					&& strncasecmp(p->server_id.s, srvid->s, srvid->len) == 0) {
				LM_DBG("TLS config found by server id\n");
				return p;
			}
		}
		if(sname) {
			LM_DBG("comparing addr: l[%s:%d]  r[%s:%d] -- sni: l[%.*s] r[%.*s] "
				   "%d"
				   " -- type: %d\n",
					ip_addr2a(&p->ip), p->port, ip_addr2a(ip), port,
					p->server_name.len, ZSW(p->server_name.s), sname->len,
					ZSW(sname->s), p->server_name_mode, p->type);
		}
		if((p->type & TLS_DOMAIN_ANY)
				|| ((p->port == 0 || p->port == port)
						&& ip_addr_cmp(&p->ip, ip))) {
			if(sname && sname->s && sname->len > 0 && p->server_name.s
					&& p->server_name.len > 0) {
				if(p->server_name_mode != KSR_TLS_SNM_SUBDOM) {
					/* match sni domain */
					if(p->server_name.len == sname->len
							&& strncasecmp(
									   p->server_name.s, sname->s, sname->len)
									   == 0) {
						LM_DBG("socket+server_name based TLS server domain "
							   "found\n");
						return p;
					}
				}
				if((p->server_name_mode == KSR_TLS_SNM_INCDOM
						   || p->server_name_mode == KSR_TLS_SNM_SUBDOM)
						&& (p->server_name.len < sname->len)) {
					dotpos = sname->len - p->server_name.len - 1;
					if(sname->s[dotpos] == '.'
							&& strncasecmp(p->server_name.s,
									   sname->s + dotpos + 1,
									   p->server_name.len)
									   == 0) {
						LM_DBG("socket+server_name based TLS server sub-domain "
							   "found\n");
						return p;
					}
				}
			} else {
				if(!(p->type & TLS_DOMAIN_ANY)) {
					LM_DBG("socket based TLS server domain found\n");
					return p;
				}
			}
		}
		p = p->next;
	}

	/* No matching domain found, return default */
	if(type & TLS_DOMAIN_SRV)
		return cfg->srv_default;
	else
		return cfg->cli_default;
}


/**
 * @brief Check whether configuration domain is duplicated
 * @param cfg configuration set
 * @param d checked domain
 * @return 1 if domain is duplicated, 0 if it's not
 */
int ksr_tls_domain_duplicated(tls_domains_cfg_t *cfg, tls_domain_t *d)
{
	tls_domain_t *p;

	if(d->type & TLS_DOMAIN_DEF) {
		if(d->type & TLS_DOMAIN_SRV) {
			if(cfg->srv_default == d) {
				return 0;
			}
			return cfg->srv_default != NULL;
		} else {
			if(cfg->cli_default == d) {
				return 0;
			}
			return cfg->cli_default != NULL;
		}
	} else {
		if(d->type & TLS_DOMAIN_SRV)
			p = cfg->srv_list;
		else
			p = cfg->cli_list;
	}

	if(d->type & TLS_DOMAIN_ANY) {
		/* any address, it must have server_name for SNI */
		if(d->server_name.len == 0) {
			LM_WARN("duplicate definition for a tls profile (same address)"
					" and no server name provided\n");
			return 1;
		} else {
			return 0;
		}
	}

	while(p) {
		if(p != d) {
			if((p->port == d->port) && ip_addr_cmp(&p->ip, &d->ip)) {
				if(d->server_name.len == 0 || p->server_name.len == 0) {
					LM_WARN("duplicate definition for a tls profile (same "
							"address)"
							" and no server name provided\n");
					return 1;
				}
			}
		}
		p = p->next;
	}

	return 0;
}


/**
 * @brief Add a domain to the configuration set
 * @param cfg configuration set
 * @param d TLS domain
 * @return 1 if domain already exists, 0 after addition, -1 on error
 */
int tls_add_domain(tls_domains_cfg_t *cfg, tls_domain_t *d)
{
	if(!cfg) {
		ERR("TLS configuration structure missing\n");
		return -1;
	}

	if(d->type & TLS_DOMAIN_DEF) {
		if(d->type & TLS_DOMAIN_CLI) {
			cfg->cli_default = d;
		} else {
			cfg->srv_default = d;
		}
	} else {
		if(d->type & TLS_DOMAIN_SRV) {
			d->next = cfg->srv_list;
			cfg->srv_list = d;
		} else {
			d->next = cfg->cli_list;
			cfg->cli_list = d;
		}
	}
	return 0;
}
