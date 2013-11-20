	/* TEST: PHY functions */
	for (i = 0; i < WORK_BUF_LEN; i++)
		send_test_work(i);
	while (1)
		read_result();
