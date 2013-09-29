#ifndef __MINER_H__
#define __MINER_H__

struct stratum_work {
	char *job_id;
	char *prev_hash;
	unsigned char **merkle_bin;
	char *bbversion;
	char *nbit;
	char *ntime;
	bool clean;

	size_t cb_len;
	size_t header_len;
	int merkles;
	double diff;
};

#endif /* __MINER_H__ */
