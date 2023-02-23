
int processFlag (int argLen, int argPos, int argc, char** argv) {
	/*
	* process flags where an argument is required (--samples and --tdelay)
	*/
	char c = argv[argPos][argLen];
	int ret = -1;

	if (c == '\0' || c == '=' || ('0'<= c && c <= '9')) {
		int adj = 0;
		if (argv[argPos][argLen] == '=') { adj = 1; }

		ret = strToInt(&argv[argPos][argLen+adj]);
		// handle cases: --samples 10  --samples= 10
		if (ret == -1 && argPos+1 < argc) {
			ret = strToInt(argv[argPos+1]);
			if (ret != -1) { return ret; }
		}
	}
	return ret;
}