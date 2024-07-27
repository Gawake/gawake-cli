#include "wday.h"

// Receives a week day from 0 to 13, and returns from 0 to 6 (Sunday to Saturday); in other words, two weeks must be represented from 0 to 6 instead of 0 to 13
int wday(int num) {
	switch(num) {
	case 0:
	case 7:
		return 0;
	case 1:
	case 8:
		return 1;
	case 2:
	case 9:
		return 2;
	case 3:
	case 10:
		return 3;
	case 4:
	case 11:
		return 4;
	case 5:
	case 12:
		return 5;
	case 6:
	case 13:
		return 6;
	}
	return -1;
}
