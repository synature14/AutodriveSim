/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001,2007 Oracle.  All rights reserved.
 *
 * $Id: rep.h,v 12.91 2007/06/22 16:42:45 bostic Exp $
 */

#ifndef _DB_REP_H_
#define	_DB_REP_H_

#include "dbinc_auto/rep_auto.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Names of client temp databases.
 */
#define	REPDBNAME	"__db.rep.db"
#define	REPPAGENAME     "__db.reppg.db"

/*
 * Message types
 */
#define	REP_INVALID	0	/* Invalid message type. */
#define	REP_ALIVE	1	/* I am alive message. */
#define	REP_ALIVE_REQ	2	/* Request for alive messages. */
#define	REP_ALL_REQ	3	/* Request all log records greater than LSN. */
#define	REP_BULK_LOG	4	/* Bulk transfer of log records. */
#define	REP_BULK_PAGE	5	/* Bulk transfer of pages. */
#define	REP_DUPMASTER	6	/* Duplicate master detected; propagate. */
#define	REP_FILE	7	/* Page of a database file. NOTUSED */
#define	REP_FILE_FAIL	8	/* File requested does not exist. */
#define	REP_FILE_REQ	9	/* Request for a database file. NOTUSED */
#define	REP_LEASE_GRANT	10	/* Client grants a lease to a master. */
#define	REP_LOG		11	/* Log record. */
#define	REP_LOG_MORE	12	/* There are more log records to request. */
#define	REP_LOG_REQ	13	/* Request for a log record. */
#define	REP_MASTER_REQ	14	/* Who is the master */
#define	REP_NEWCLIENT	15	/* Announces the presence of a new client. */
#define	REP_NEWFILE	16	/* Announce a log file change. */
#define	REP_NEWMASTER	17	/* Announces who the master is. */
#define	REP_NEWSITE	18	/* Announces that a site has heard from a new
				 * site; like NEWCLIENT, but indirect.  A
				 * NEWCLIENT message comes directly from the new
				 * client while a NEWSITE comes indirectly from
				 * someone who heard about a NEWSITE.
				 */
#define	REP_PAGE	19	/* Database page. */
#define	REP_PAGE_FAIL	20	/* Requested page does not exist. */
#define	REP_PAGE_MORE	21	/* There are more pages to request. */
#define	REP_PAGE_REQ	22	/* Request for a database page. */
#define	REP_REREQUEST	23	/* Force rerequest. */
#define	REP_START_SYNC	24	/* Tell client to begin syncing a ckp.*/
#define	REP_UPDATE	25	/* Environment hotcopy information. */
#define	REP_UPDATE_REQ	26	/* Request for hotcopy information. */
#define	REP_VERIFY	27	/* A log record for verification. */
#define	REP_VERIFY_FAIL	28	/* The client is outdated. */
#define	REP_VERIFY_REQ	29	/* Request for a log record to verify. */
#define	REP_VOTE1	30	/* Send out your information for an election. */
#define	REP_VOTE2	31	/* Send a "you are master" vote. */
/*
 * Maximum message number for conversion tables.  Update this
 * value as the largest message number above increases.
 *
 * !!!
 * NOTE: When changing messages above, the two tables for upgrade support
 * need adjusting.  They are in rep_util.c.
 */
#define	REP_MAX_MSG	31

/*
 * This is the list of client-to-client requests messages.
 * We use this to decide if we're doing client-to-client and
 * might need to send a rerequest.
 */
#define	REP_MSG_REQ(rectype)			\
    (rectype == REP_ALL_REQ ||			\
    rectype == REP_LOG_REQ ||			\
    rectype == REP_PAGE_REQ ||			\
    rectype == REP_VERIFY_REQ)

/*
 * Note that the version information should be at the beginning of the
 * structure, so that we can rearrange the rest of it while letting the
 * version checks continue to work.  DB_REPVERSION should be revved any time
 * the rest of the structure changes or when the message numbers change.
 *
 * Define also, the corresponding log versions that are tied to the
 * replication/release versions.  These are only used in replication
 * and that is why they're defined here.
 */
#define	DB_LOGVERSION_42	8
#define	DB_LOGVERSION_43	10
#define	DB_LOGVERSION_44	11
#define	DB_LOGVERSION_45	12
#define	DB_LOGVERSION_46	13
#define	DB_REPVERSION_INVALID	0
#define	DB_REPVERSION_42	1
#define	DB_REPVERSION_43	2
#define	DB_REPVERSION_44	3
#define	DB_REPVERSION_45	3
#define	DB_REPVERSION_46	4
#define	DB_REPVERSION		DB_REPVERSION_46

/*
 * RPRINT
 * REP_PRINT_MESSAGE
 *	Macros for verbose replication messages.
 */
#define	RPRINT(dbenv, x) do {						\
	if (FLD_ISSET((dbenv)->verbose, DB_VERB_REPLICATION)) {		\
		__rep_print x;						\
	}								\
} while (0)
#define	REP_PRINT_MESSAGE(dbenv, eid, rp, str, fl) do {			\
	if (FLD_ISSET((dbenv)->verbose, DB_VERB_REPLICATION)) {		\
		__rep_print_message(dbenv, eid, rp, str, fl);		\
	}								\
} while (0)

/*
 * Election gen file name
 * The file contains an egen number for an election this client has NOT
 * participated in.  I.e. it is the number of a future election.  We
 * create it when we create the rep region, if it doesn't already exist
 * and initialize egen to 1.  If it does exist, we read it when we create
 * the rep region.  We write it immediately before sending our VOTE1 in
 * an election.  That way, if a client has ever sent a vote for any
 * election, the file is already going to be updated to reflect a future
 * election, should it crash.
 */
#define	REP_EGENNAME	"__db.rep.egen"
#define	REP_GENNAME	"__db.rep.gen"

/*
 * Internal init flag file name:
 * The existence of this file serves as an indication that the client is in the
 * process of Internal Initialization, in case it crashes before completing.
 * During internal init the client's partially reconstructed database pages and
 * logs may be in an inconsistent state, so much so that running recovery must
 * be avoided.  Furthermore, there is no other way to reliably recognize this
 * condition.  Therefore, when we open an environment, and we're just about to
 * run recovery, we check for this file first.  If it exists we must discard all
 * logs and databases.  This avoids the recovery problems, and leads to a fresh
 * attempt at internal init if the environment becomes a replication client and
 * finds a master.  The list of databases which may need to be removed is stored
 * in this file.
 */
#define	REP_INITNAME	"__db.rep.init"

/*
 * Database types for __rep_client_dbinit
 */
typedef enum {
	REP_DB,		/* Log record database. */
	REP_PG		/* Pg database. */
} repdb_t;

/* Macros to lock/unlock the replication region as a whole. */
#define	REP_SYSTEM_LOCK(dbenv)						\
	MUTEX_LOCK(dbenv, (dbenv)->rep_handle->region->mtx_region)
#define	REP_SYSTEM_UNLOCK(dbenv)					\
	MUTEX_UNLOCK(dbenv, (dbenv)->rep_handle->region->mtx_region)

/*
 * Macros for manipulating the event synchronization.  We use a separate mutex
 * so that an application's call-back function can be invoked without locking
 * the whole region.
 */
#define	REP_EVENT_LOCK(dbenv)					     \
	MUTEX_LOCK(dbenv, (dbenv)->rep_handle->region->mtx_event)
#define	REP_EVENT_UNLOCK(dbenv)					     \
	MUTEX_UNLOCK(dbenv, (dbenv)->rep_handle->region->mtx_event)

/*
 * REP --
 * Shared replication structure.
 */
typedef struct __rep {
	db_mutex_t	mtx_region;	/* Region mutex. */
	db_mutex_t	mtx_clientdb;	/* Client database mutex. */
	db_mutex_t	mtx_ckp;	/* Checkpoint mutex. */
	roff_t		lease_off;	/* Offset of the lease table. */
	roff_t		tally_off;	/* Offset of the tally region. */
	roff_t		v2tally_off;	/* Offset of the vote2 tally region. */
	int		eid;		/* Environment id. */
	int		master_id;	/* ID of the master site. */
	u_int32_t	version;	/* Current replication version. */
	u_int32_t	egen;		/* Replication election generation. */
	u_int32_t	gen;		/* Replication generation number. */
	u_int32_t	recover_gen;	/* Last generation number in log. */
	int		asites;		/* Space allocated for sites. */
	int		nsites;		/* Number of sites in group. */
	int		nvotes;		/* Number of votes needed. */
	int		priority;	/* My priority in an election. */
	int		config_nsites;

	db_timeout_t	elect_timeout;	/* Normal/full election timeouts. */
	db_timeout_t	full_elect_timeout;

	db_timeout_t	chkpt_delay;	/* Master checkpoint delay. */

#define	REP_DEFAULT_THROTTLE	(10 * MEGABYTE) /* Default value is < 1Gig. */
	u_int32_t	gbytes;		/* Limit on data sent in single... */
	u_int32_t	bytes;		/* __rep_process_message call. */
#define	DB_REP_REQUEST_GAP	4
#define	DB_REP_MAX_GAP		128
	u_int32_t	request_gap;	/* # of records to receive before we
					 * request a missing log record. */
	u_int32_t	max_gap;	/* Maximum number of records before
					 * requesting a missing log record. */
	/* Status change information */
	u_int32_t	apply_th;	/* Number of callers in rep_apply. */
	u_int32_t	msg_th;		/* Number of callers in rep_proc_msg.*/
	u_int32_t	handle_cnt;	/* Count of handles in library. */
	u_int32_t	op_cnt;		/* Multi-step operation count.*/
	DB_LSN		ckp_lsn;	/* LSN for syncing a checkpoint. */
	DB_LSN		max_prep_lsn;	/* Max LSN of txn_prepare record. */

	/*
	 * Event notification synchronization: the mtx_event and associate
	 * fields which it protects govern event notification to the
	 * application.  They form a guarantee that no matter how crazy the
	 * thread scheduling gets, the application sees a sensible, orderly
	 * progression of events.
	 */
	db_mutex_t	mtx_event;	/* Serializes event notification. */
	/*
	 * Latest generation whose NEWMASTER event the application has been
	 * notified of.  Also serves to force STARTUPDONE to occur after
	 * NEWMASTER.
	 */
	u_int32_t	newmaster_event_gen;
	/*
	 * Latest local victory of an election that the application has been
	 * notified of, expressed as the election generation number.  This
	 * ensures we notify the application exactly once when it wins an
	 * election.
	 */
	u_int32_t	notified_egen;

	/* Backup information. */
	u_int32_t	nfiles;		/* Number of files we have info on. */
	u_int32_t	curfile;	/* Cur file we're getting (0-based). */
	__rep_fileinfo_args	*curinfo;	/* Current file info ptr. */
	void		*finfo;		/* Current file info buffer. */
	void		*nextinfo;	/* Next file info buffer. */
	void		*originfo;	/* Original file info buffer. */
	DB_LSN		first_lsn;	/* Earliest LSN we need. */
	u_int32_t	first_vers;	/* Log version of first log file. */
	DB_LSN		last_lsn;	/* Latest LSN we need. */
	db_pgno_t	ready_pg;	/* Next pg expected. */
	db_pgno_t	waiting_pg;	/* First pg after gap. */
	db_pgno_t	max_wait_pg;	/* Maximum pg requested. */
	u_int32_t	npages;		/* Num of pages rcvd for this file. */
	DB_MPOOLFILE	*file_mpf;	/* Mpoolfile for in-mem database. */
	DB		*file_dbp;	/* This file's page info. */
	DB		*queue_dbp;	/* Dbp for a queue file. */

	/* Vote tallying information. */
	int		sites;		/* Sites heard from. */
	int		winner;		/* Current winner. */
	int		w_priority;	/* Winner priority. */
	u_int32_t	w_gen;		/* Winner generation. */
	DB_LSN		w_lsn;		/* Winner LSN. */
	u_int32_t	w_tiebreaker;	/* Winner tiebreaking value. */
	int		votes;		/* Number of votes for this site. */

	db_timespec	etime;		/* Election start timestamp. */

	/* Leases. */
	db_timeout_t	lease_timeout;	/* Lease timeout. */
	db_timespec	lease_duration;	/* Lease timeout with clock skew. */
	u_int32_t	clock_skew;	/* Clock skew factor. */
	db_timespec	grant_expire;	/* Local grant expiration time. */

	/* Statistics. */
	DB_REP_STAT	stat;
#if defined(HAVE_REPLICATION_THREADS) && defined(HAVE_STATISTICS)
	DB_REPMGR_STAT	mstat;
#endif

	/* Configuration. */
#define	REP_C_BULK		0x00001		/* Bulk transfer. */
#define	REP_C_DELAYCLIENT	0x00002		/* Delay client sync-up. */
#define	REP_C_LEASE		0x00004		/* Leases configured. */
#define	REP_C_NOAUTOINIT	0x00008		/* No auto initialization. */
#define	REP_C_NOWAIT		0x00010		/* Immediate error return. */
	u_int32_t	config;		/* Configuration flags. */

#define	REP_F_CLIENT		0x00000001	/* Client replica. */
#define	REP_F_DELAY		0x00000002	/* Delaying client sync-up. */
#define	REP_F_EGENUPDATE	0x00000004	/* Egen updated by ALIVE msg. */
#define	REP_F_EPHASE0		0x00000008	/* In phase 0 of election. */
#define	REP_F_EPHASE1		0x00000010	/* In phase 1 of election. */
#define	REP_F_EPHASE2		0x00000020	/* In phase 2 of election. */
#define	REP_F_GROUP_ESTD	0x00000040	/* Rep group is established. */
#define	REP_F_INREPELECT	0x00000080	/* Thread in rep_elect. */
#define	REP_F_MASTER		0x00000100	/* Master replica. */
#define	REP_F_MASTERELECT	0x00000200	/* Master elect. */
#define	REP_F_NEWFILE		0x00000400	/* Newfile in progress. */
#define	REP_F_NOARCHIVE		0x00000800	/* Rep blocks log_archive. */
#define	REP_F_READY_API		0x00001000	/* Need handle_cnt to be 0. */
#define	REP_F_READY_APPLY	0x00002000	/* Need apply_th to be 0. */
#define	REP_F_READY_MSG		0x00004000	/* Need msg_th to be 0. */
#define	REP_F_READY_OP		0x00008000	/* Need op_cnt to be 0. */
#define	REP_F_RECOVER_LOG	0x00010000	/* In recovery - log. */
#define	REP_F_RECOVER_PAGE	0x00020000	/* In recovery - pages. */
#define	REP_F_RECOVER_UPDATE	0x00040000	/* In recovery - files. */
#define	REP_F_RECOVER_VERIFY	0x00080000	/* In recovery - verify. */
#define	REP_F_SKIPPED_APPLY	0x00100000	/* Skipped applying a record. */
#define	REP_F_START_CALLED	0x00200000	/* Rep_start called. */
#define	REP_F_TALLY		0x00400000	/* Tallied vote before elect. */
	u_int32_t	flags;
} REP;

/*
 * Recovery flag mask to easily check any/all recovery bits.  That is
 * REP_F_READY_{API|OP} and all REP_F_RECOVER*.  This must change if the values
 * of the flags change.  NOTE:  We do not include REP_F_READY_MSG in
 * this mask because it is used frequently in non-recovery related
 * areas and we want to manipulate it separately (see especially
 * in __rep_new_master).
 */
#define	REP_F_RECOVER_MASK						\
    (REP_F_READY_API | REP_F_READY_OP |					\
     REP_F_RECOVER_LOG | REP_F_RECOVER_PAGE |				\
     REP_F_RECOVER_UPDATE | REP_F_RECOVER_VERIFY)

/*
 * REP_F_EPHASE0 is not a *real* election phase.  It is used for
 * master leases and allowing the client to find the master or
 * expire its lease.  However, EPHASE0 is cleared by __rep_elect_done.
 */
#define	IN_ELECTION(R)							\
	F_ISSET((R), REP_F_EPHASE1 | REP_F_EPHASE2)
#define	IN_ELECTION_TALLY(R) \
	F_ISSET((R), REP_F_EPHASE1 | REP_F_EPHASE2 | REP_F_TALLY)
#define	ELECTION_MAJORITY(n) (((n) / 2) + 1)

#define	REP_F_INTERNAL_INIT_MASK (REP_F_RECOVER_PAGE | REP_F_RECOVER_LOG)

#define	IS_REP_MASTER(dbenv)						\
	(REP_ON(dbenv) &&						\
	    F_ISSET(((REP *)(dbenv)->rep_handle->region), REP_F_MASTER))

#define	IS_REP_CLIENT(dbenv)						\
	(REP_ON(dbenv) &&						\
	    F_ISSET(((REP *)(dbenv)->rep_handle->region), REP_F_CLIENT))

#define	IS_USING_LEASES(dbenv)						\
	(REP_ON(dbenv) &&						\
	    FLD_ISSET(((REP *)(dbenv)->rep_handle->region)->config,	\
	    REP_C_LEASE))

#define	IS_CLIENT_PGRECOVER(dbenv)					\
	(IS_REP_CLIENT(dbenv) &&					\
	    F_ISSET(((REP *)(dbenv)->rep_handle->region), REP_F_RECOVER_PAGE))

/*
 * Macros to figure out if we need to do replication pre/post-amble processing.
 * Skip for specific DB handles owned by the replication layer, either because
 * replication is running recovery or because it's a handle entirely owned by
 * the replication code (replication opens its own databases to track state).
 */
#define	IS_ENV_REPLICATED(dbenv)					\
	(REP_ON(dbenv) && (dbenv)->rep_handle->region->flags != 0)

/*
 * Gap processing flags.  These provide control over the basic
 * gap processing algorithm for some special cases.
 */
#define	REP_GAP_FORCE		0x001	/* Force a request for a gap. */
#define	REP_GAP_REREQUEST	0x002	/* Gap request is a forced rerequest. */
					/* REREQUEST is a superset of FORCE. */

/*
 * Basic pre/post-amble processing.
 */
#define	REPLICATION_WRAP(dbenv, func_call, ret) do {			\
	int __rep_check, __t_ret;					\
	__rep_check = IS_ENV_REPLICATED(dbenv) ? 1 : 0;			\
	if (__rep_check && ((ret) = __env_rep_enter(dbenv, 0)) != 0)	\
		return ((ret));						\
	(ret) = func_call;						\
	if (__rep_check &&						\
	    (__t_ret = __env_db_rep_exit(dbenv)) != 0 && (ret) == 0)	\
		(ret) = __t_ret;					\
} while (0)

/*
 * Per-process replication structure.
 *
 * There are 2 mutexes used in replication.
 * 1.  mtx_region - This protects the fields of the rep region above.
 * 2.  mtx_clientdb - This protects the per-process flags, and bookkeeping
 * database and all of the components that maintain it.  Those
 * components include the following fields in the log region (see log.h):
 *	a. ready_lsn
 *	b. waiting_lsn
 *	c. verify_lsn
 *	d. wait_recs
 *	e. rcvd_recs
 *	f. max_wait_lsn
 * These fields in the log region are NOT protected by the log region lock at
 * all.
 *
 * Note that the per-process flags should truly be protected by a special
 * per-process thread mutex, but it is currently set in so isolated a manner
 * that it didn't make sense to do so and in most case we're already holding
 * the mtx_clientdb anyway.
 *
 * The lock ordering protocol is that mtx_clientdb must be acquired first and
 * then either REP->mtx_region, or the LOG->mtx_region mutex may be acquired if
 * necessary.
 */
struct __db_rep {
	/*
	 * Shared configuration information -- copied to and maintained in the
	 * shared region as soon as the shared region is created.
	 */
	int		eid;		/* Environment ID. */

	u_int32_t	gbytes;		/* Limit on data sent in single... */
	u_int32_t	bytes;		/* __rep_process_message call. */

	u_int32_t	request_gap;	/* # of records to receive before we
					 * request a missing log record. */
	u_int32_t	max_gap;	/* Maximum number of records before
					 * requesting a missing log record. */

	u_int32_t	clock_skew;	/* Clock skew factor. */
	u_int32_t	config;		/* Configuration flags. */
	int		config_nsites;

	db_timeout_t	elect_timeout;	/* Normal/full election timeouts. */
	db_timeout_t	full_elect_timeout;

	db_timeout_t	chkpt_delay;	/* Master checkpoint delay. */

	int		my_priority;
	db_timeout_t	lease_timeout;	/* Master leases. */
	/*
	 * End of shared configuration information.
	 */
	int		(*send)		/* Send function. */
			    __P((DB_ENV *, const DBT *, const DBT *,
			    const DB_LSN *, int, u_int32_t));

	DB		*rep_db;	/* Bookkeeping database. */

	REP		*region;	/* In memory structure. */
	u_int8_t	*bulk;		/* Shared memory bulk area. */

#define	DBREP_OPENFILES		0x0001	/* This handle has opened files. */
	u_int32_t	flags;		/* per-process flags. */

#ifdef HAVE_REPLICATION_THREADS
	/*
	 * Replication Framework (repmgr) information.
	 */
	int		nthreads;
	u_int32_t	init_policy;
	int		perm_policy;
	int		peer;	/* Site to use for C2C sync. */
	db_timeout_t	ack_timeout;
	db_timeout_t	election_retry_wait;
	db_timeout_t	connection_retry_wait;

	/* Repmgr's copies of rep stuff. */
	int		master_eid;
	u_int32_t	generation;

	/* Thread synchronization. */
	REPMGR_RUNNABLE *selector, **messengers, *elect_thread;
	mgr_mutex_t	mutex;
	cond_var_t	queue_nonempty, check_election;
#ifdef DB_WIN32
	ACK_WAITERS_TABLE *waiters;
	HANDLE		signaler;
	int		wsa_inited;
#else
	pthread_cond_t	ack_condition;
	int		read_pipe, write_pipe;
	int		chg_sig_handler;
#endif

	/* Operational stuff. */
	REPMGR_SITE	*sites;		/* Array of known sites. */
	u_int		site_cnt;	/* Array slots in use. */
	u_int		site_max;	/* Total array slots allocated. */

	CONNECTION_LIST	connections;
	RETRY_Q_HEADER	retries;	/* Sites needing connection retry. */
	REPMGR_QUEUE	*input_queue;

	socket_t	listen_fd;
	repmgr_netaddr_t my_addr;

	int		finished; /* Repmgr threads should shut down. */
	int		done_one; /* TODO: rename */
	int		found_master;
	int		takeover_pending; /* We've been elected master. */

/* Operations we can ask election thread to perform (OOB value is 0): */
#define	ELECT_ELECTION		1 /* Call for an election. */
#define	ELECT_FAILURE_ELECTION	2 /* Do election, adjusting nsites to account
				     for a failed master. */
#define	ELECT_REPSTART		3 /* Call rep_start(CLIENT). */
#define	ELECT_SEEK_MASTER	4 /* Alternate rep_start to find master. */
	int		operation_needed; /* Next op for election thread. */

#endif  /* HAVE_REPLICATION_THREADS */
};

/*
 * Control structure for replication communication infrastructure.
 */
typedef struct {
	u_int32_t	rep_version;	/* Replication version number. */
	u_int32_t	log_version;	/* Log version number. */

	DB_LSN		lsn;		/* Log sequence number. */
	u_int32_t	rectype;	/* Message type. */
	u_int32_t	gen;		/* Generation number. */
	db_timespec	msg_time;	/* Timestamp for leases. */
/*
 * Define old DB_LOG_ values that we must support here.  For reasons of
 * compatibility with old versions, these values must be reserved explicitly in
 * the list of flag values (below)
 */
#define	DB_LOG_PERM_42_44	0x20
#define	DB_LOG_RESEND_42_44	0x40

#define	REPCTL_ELECTABLE	0x01	/* Upgraded client is electable. */
#define	REPCTL_FLUSH		0x02	/* Record should be flushed. */
#define	REPCTL_GROUP_ESTD	0x04	/* Message from site in a group. */
#define	REPCTL_INIT		0x08	/* Internal init message. */
#define	REPCTL_LEASE		0x10	/* Lease related message.. */
			/*
			 * Skip over reserved values 0x20
			 * and 0x40, as explained above.
			 */
#define	REPCTL_LOG_END		0x80	/* Approximate end of group-wide log. */
#define	REPCTL_PERM		DB_LOG_PERM_42_44
#define	REPCTL_RESEND		DB_LOG_RESEND_42_44
	u_int32_t	flags;		/* log_put flag value. */
} REP_CONTROL;

/*
 * Control message format for 4.5 release and earlier.
 */
typedef struct {
	u_int32_t	rep_version;	/* Replication version number. */
	u_int32_t	log_version;	/* Log version number. */

	DB_LSN		lsn;		/* Log sequence number. */
	u_int32_t	rectype;	/* Message type. */
	u_int32_t	gen;		/* Generation number. */
	u_int32_t	flags;		/* log_put flag value. */
} REP_OLD_CONTROL;

#define	LEASE_REFRESH_TRIES	3	/* Number of times to try refresh. */

/* Master granted lease information. */
typedef struct __rep_lease_entry {
	int		eid;		/* EID of client grantor. */
	db_timespec	start_time;	/* Start time clients echo back. */
	db_timespec	end_time;	/* Master lease expiration time. */
	DB_LSN		lease_lsn;	/* Durable LSN lease applies to. */
} REP_LEASE_ENTRY;

typedef struct __rep_grant_info {
	db_timespec	msg_time;	/* Echoed master timestamp. */
#ifdef	DIAGNOSTIC
	db_timespec	expire_time;	/* Local expiration time. */
#endif
} REP_GRANT_INFO;

/* Election vote information, 4.2 version.  Does not have nvotes. */
typedef struct __rep_old_vote {
	u_int32_t	egen;		/* Election generation. */
	int		nsites;		/* Number of sites I've been in
					 * communication with. */
	int		priority;	/* My site's priority. */
	u_int32_t	tiebreaker;	/* Tie-breaking quasi-random value. */
} REP_OLD_VOTE_INFO;

/* Election vote information. */
typedef struct {
	u_int32_t	egen;		/* Election generation. */
	int		nsites;		/* Number of sites I've been in
					 * communication with. */
	int		nvotes;		/* Number of votes needed to win. */
	int		priority;	/* My site's priority. */
	u_int32_t	tiebreaker;	/* Tie-breaking quasi-random value. */
} REP_VOTE_INFO;

typedef struct {
	u_int32_t	egen;		/* Voter's election generation. */
	int		eid;		/* Voter's ID. */
} REP_VTALLY;

/*
 * The REP_THROTTLE_ONLY flag is used to do throttle processing only.
 * If set, it will only allow sending the REP_*_MORE message, but not
 * the normal, non-throttled message.  It is used to support throttling
 * with bulk transfer.
 */
/* Flags for __rep_send_throttle. */
#define	REP_THROTTLE_ONLY	0x0001	/* Send _MORE message only. */

/* Throttled message processing information. */
typedef struct {
	DB_LSN		lsn;		/* LSN of this record. */
	DBT		*data_dbt;	/* DBT of this record. */
	u_int32_t	gbytes;		/* This call's max gbytes sent. */
	u_int32_t	bytes;		/* This call's max bytes sent. */
	u_int32_t	type;		/* Record type. */
} REP_THROTTLE;

/* Bulk processing information. */
/*
 * !!!
 * We use a uintptr_t for the offset.  We'd really like to use a ptrdiff_t
 * since that really is what it is.  But ptrdiff_t is not portable and
 * doesn't exist everywhere.
 */
typedef struct {
	u_int8_t	*addr;		/* Address of bulk buffer. */
	uintptr_t	*offp;		/* Ptr to current offset into buffer. */
	u_int32_t	len;		/* Bulk buffer length. */
	u_int32_t	type;		/* Item type in buffer (log, page). */
	DB_LSN		lsn;		/* First LSN in buffer. */
	int		eid;		/* ID of potential recipients. */
#define	BULK_XMIT	0x001		/* Buffer in transit. */
	u_int32_t	*flagsp;	/* Buffer flags. */
} REP_BULK;

/*
 * This structure takes care of representing a transaction.
 * It holds all the records, sorted by page number so that
 * we can obtain locks and apply updates in a deadlock free
 * order.
 */
typedef struct {
	u_int nlsns;
	u_int nalloc;
	DB_LSN *array;
} LSN_COLLECTION;

/*
 * This is used by the page-prep routines to do the lock_vec call to
 * apply the updates for a single transaction or a collection of
 * transactions.
 */
typedef struct {
	int		n;
	DB_LOCKREQ	*reqs;
	DBT		*objs;
} linfo_t;

#if defined(__cplusplus)
}
#endif

#include "dbinc_auto/rep_ext.h"
#endif	/* !_DB_REP_H_ */
