#pragma once

enum class Algoritm { crc32, GOST };
enum class Change { N, Y, New };
enum class Point { One, All, Next, Back, Exit };

typedef struct {

	vect hash_buf;
	unsigned long hash_crc32;

}Hash;