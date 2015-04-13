int x;
int y;

void main() {
	x, y = 5, 13;
	swap();
	assert(x > y);
}

void swap() {
	x = x + y;
	y = x - y;
	x = x - y;
}

// int param;
// int result;

// void main() {
//	param = 1;
//	fun();
//	assert(result > 0);
// }

// void fun() {
//	int local;
//	if (param <= 0) {
//		result = 1;
//	} else {
//		local = param;
//		param = param - 1;
//		fun();
//		result = result + local;
//	}
// }
