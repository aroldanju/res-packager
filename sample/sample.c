#include "res.h"

#include <stdio.h>

int main(int argc, char* argv[]) {
	RES res;

	printf("Res Packager Sample\n");

	// Pack some files
	if (res_create(&res, "RES.TXT", "./") != 0) {
		printf("Error creating package.\n");
		return 1;
	}

	res_save(&res, "PACK.DAT");

	res_dispose(&res);

	// Load a package and shows the content of second file
	res_load(&res, "./PACK.DAT");

	char *data = res_get_file(&res, 1);
	printf("%s", data);
	res_dispose(&res);

	return 0;
}
