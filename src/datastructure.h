/* Pi-hole: A black hole for Internet advertisements
*  (c) 2019 Pi-hole, LLC (https://pi-hole.net)
*  Network-wide ad blocking via your own hardware.
*
*  FTL Engine
*  Datastructure prototypes
*
*  This file is copyright under the latest version of the EUPL.
*  Please see LICENSE file for your rights under this license. */
#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

// Definition of sqlite3_stmt
#include "database/sqlite3.h"

// enum privacy_level
#include "enums.h"

// assert_sizeof
#include "static_assert.h"

typedef struct {
	unsigned char magic;
	enum query_status status;
	enum query_types type;
	enum privacy_level privacylevel;
	enum reply_type reply;
	enum dnssec_status dnssec;
	uint16_t qtype;
	int domainID;
	int clientID;
	int upstreamID;
	int id; // the ID is a (signed) int in dnsmasq, so no need for a long int here
	int CNAME_domainID; // only valid if query has a CNAME blocking status
	unsigned int timeidx;
	unsigned long response; // saved in units of 1/10 milliseconds (1 = 0.1ms, 2 = 0.2ms, 2500 = 250.0ms, etc.)
	unsigned long forwardresponse; // saved in units of 1/10 milliseconds (1 = 0.1ms, 2 = 0.2ms, 2500 = 250.0ms, etc.)
	double timestamp;
	int64_t db;
	// Adjacent bit field members in the struct flags may be packed to share
	// and straddle the individual bytes. It is useful to pack the memory as
	// tightly as possible as there may be dozens of thousands of these
	// objects in memory (one per query).
	// C99 guarentees that bit-fields will be packed as tightly as possible,
	// provided they don’t cross storageau unit boundaries (6.7.2.1 #10).
	struct query_flags {
		bool whitelisted :1;
		bool complete :1;
		bool blocked :1;
	} flags;
} queriesData;

// ARM needs alignment to 8-byte boundary
ASSERT_SIZEOF(queriesData, 72, 60, 64);

typedef struct {
	unsigned char magic;
	struct upstream_flags {
		bool new:1;
	} flags;
	in_port_t port;
	int count;
	int failed;
	unsigned int responses;
	unsigned long rtime;
	unsigned long rtuncertainty;
	size_t ippos;
	size_t namepos;
	double lastQuery;
} upstreamsData;
ASSERT_SIZEOF(upstreamsData, 56, 40, 40);

typedef struct {
	unsigned char magic;
	unsigned char reread_groups;
	char hwlen;
	unsigned char hwaddr[16]; // See DHCP_CHADDR_MAX in dnsmasq/dhcp-protocol.h
	struct client_flags {
		bool new:1;
		bool found_group:1;
		bool aliasclient:1;
	} flags;
	int count;
	int blockedcount;
	int aliasclient_id;
	unsigned int id;
	unsigned int rate_limit;
	unsigned int numQueriesARP;
	int overTime[OVERTIME_SLOTS];
	size_t groupspos;
	size_t ippos;
	size_t namepos;
	size_t ifacepos;
	time_t firstSeen;
	double lastQuery;
} clientsData;

// ARM needs alignment to 8-byte boundary
ASSERT_SIZEOF(clientsData, 696, 672, 672);

typedef struct {
	unsigned char magic;
	int count;
	int blockedcount;
	size_t domainpos;
} domainsData;
ASSERT_SIZEOF(domainsData, 24, 16, 16);

typedef struct {
	unsigned char magic;
	enum domain_client_status blocking_status;
	unsigned char force_reply;
	enum query_types query_type;
	int domainID;
	int clientID;
	int deny_regex_id;
} DNSCacheData;
ASSERT_SIZEOF(DNSCacheData, 16, 16, 16);

void strtolower(char *str);
int findQueryID(const int id);
int findUpstreamID(const char * upstream, const in_port_t port);
int findDomainID(const char *domain, const bool count);
int findClientID(const char *client, const bool count, const bool aliasclient);
int findCacheID(int domainID, int clientID, enum query_types query_type);
bool isValidIPv4(const char *addr);
bool isValidIPv6(const char *addr);

void FTL_reload_all_domainlists(void);
void FTL_reset_per_client_domain_data(void);

const char *getDomainString(const queriesData* query);
const char *getCNAMEDomainString(const queriesData* query);
const char *getClientIPString(const queriesData* query);
const char *getClientNameString(const queriesData* query);

void change_clientcount(clientsData *client, int total, int blocked, int overTimeIdx, int overTimeMod);
const char *get_query_type_str(const queriesData *query, char *buffer);
const char *get_query_status_str(const queriesData *query) __attribute__ ((pure));
const char *get_query_dnssec_str(const queriesData *query) __attribute__ ((pure));
const char *get_query_reply_str(const queriesData *query) __attribute__ ((pure));

// Pointer getter functions
#define getQuery(queryID, checkMagic) _getQuery(queryID, checkMagic, __LINE__, __FUNCTION__, __FILE__)
queriesData* _getQuery(int queryID, bool checkMagic, int line, const char * function, const char * file);
#define getClient(clientID, checkMagic) _getClient(clientID, checkMagic, __LINE__, __FUNCTION__, __FILE__)
clientsData* _getClient(int clientID, bool checkMagic, int line, const char * function, const char * file);
#define getDomain(domainID, checkMagic) _getDomain(domainID, checkMagic, __LINE__, __FUNCTION__, __FILE__)
domainsData* _getDomain(int domainID, bool checkMagic, int line, const char * function, const char * file);
#define getUpstream(upstreamID, checkMagic) _getUpstream(upstreamID, checkMagic, __LINE__, __FUNCTION__, __FILE__)
upstreamsData* _getUpstream(int upstreamID, bool checkMagic, int line, const char * function, const char * file);
#define getDNSCache(cacheID, checkMagic) _getDNSCache(cacheID, checkMagic, __LINE__, __FUNCTION__, __FILE__)
DNSCacheData* _getDNSCache(int cacheID, bool checkMagic, int line, const char * function, const char * file);

#endif //DATASTRUCTURE_H
