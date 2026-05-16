///////////////////////////////////////////////////////////////////////////
// 
//  Copyright 2026 by Pavel Chistyakov
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "print.h"
#include "stringbuilder.h"

fnoutput print_proc;

void printv(const char* format, const char* format_param) {
	if(print_proc)
		print_proc(format, format_param);
}

void print(const char* format, ...) {
	XVA_FORMAT(format);
	printv(format, format_param);
}

void println() {
	printv("\r\n", 0);
}

void println(const char* format, ...) {
	XVA_FORMAT(format);
	printv(format, format_param);
	printv("\r\n", 0);
}