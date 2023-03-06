#include "GOSTHash.h"

static void GOSTHashX(const uint8_t* a, const uint8_t* b, uint8_t* c) {

	for (int i = 0; i < 64; i++)
		c[i] = a[i] ^ b[i];

}

static void GOSTHashAdd512(const uint8_t* a, const uint8_t* b, uint8_t* c) {

	int internal = 0;

	for (int i = 0; i < 64; i++)
	{
		internal = a[i] + b[i] + (internal >> 8);

		c[i] = internal & 0xff;
	}
}

static void GOSTHashS(uint8_t* state) {

	vect internal;

	for (int i = 63; i >= 0; i--)
		internal[i] = Pi[state[i]];

	memcpy(state, internal, BLOCK_SIZE);

}

static void GOSTHashP(uint8_t* state) {

	vect internal;

	for (int i = 63; i >= 0; i--) {
		internal[i] = state[Tau[i]];
	}

	memcpy(state, internal, BLOCK_SIZE);

}

static void GOSTHashL(uint8_t* state) {

	// Делим исходный вектор на восьмибайтовые порции
	uint64_t* internal_in = (uint64_t*)state;
	uint64_t internal_out[8];
	memset(internal_out, 0x00, BLOCK_SIZE);
	int i, j;
	for (i = 7; i >= 0; i--)
	{
		// Если очередной бит равен 1, то ксорим очередное
		// значение матрицы A с предыдущими
		for (j = 63; j >= 0; j--)
			if ((internal_in[i] >> j) & 1)
				internal_out[i] ^= A[63 - j];
	}
	memcpy(state, internal_out, 64);

}

static void GOSTHashGetKey(uint8_t* K, int i) {

	GOSTHashX(K, C[i], K);
	GOSTHashS(K);
	GOSTHashP(K);
	GOSTHashL(K);

}

static void GOSTHashE(uint8_t* K, const uint8_t* m, uint8_t* state) {

	int i;
	memcpy(K, K, BLOCK_SIZE);
	GOSTHashX(m, K, state);
	for (i = 0; i < 12; i++)
	{
		GOSTHashS(state);
		GOSTHashP(state);
		GOSTHashL(state);
		GOSTHashGetKey(K, i);
		GOSTHashX(state, K, state);
	}

}

static void GOSTHashG(uint8_t* h, uint8_t* N, const uint8_t* m) {

	vect K, internal;
	GOSTHashX(N, h, K);

	GOSTHashS(K);
	GOSTHashP(K);
	GOSTHashL(K);

	GOSTHashE(K, m, internal);

	GOSTHashX(internal, h, internal);
	GOSTHashX(internal, m, h);

}

static void GOSTHashPadding(TGOSTHashContext* CTX) {

	vect internal; // Промежуточный вектор
	if (CTX->buf_size < BLOCK_SIZE)
	{
		memset(internal, 0x00, BLOCK_SIZE); // Обнуляем промежуточный вектор
		memcpy(internal, CTX->buffer, CTX->buf_size); // Пишем остаток сообщения
													  // в промежуточный вектор
		internal[CTX->buf_size] = 0x01; // Добавляем в нужное место единичку
		memcpy(CTX->buffer, internal, BLOCK_SIZE); // Кладем все, что получилось, обратно
	}

}


void GOSTHashInit(TGOSTHashContext* CTX, uint16_t hash_size) {

	memset(CTX, 0x00, sizeof(TGOSTHashContext)); // Обнуляем все переменные
											  // базовой структуры
	if (hash_size == 256)
		memset(CTX->h, 0x01, BLOCK_SIZE); // Длина хеша 256 бит
	else
		memset(CTX->h, 0x00, BLOCK_SIZE); // Длина хеша 512 бит
	CTX->hash_size = hash_size;
	CTX->v_512[1] = 0x02; // Инициализируем вектор v_512

}

static void GOSTHashStage_2(TGOSTHashContext* CTX, const uint8_t* data) {

	GOSTHashG(CTX->h, CTX->N, data);
	GOSTHashAdd512(CTX->N, CTX->v_512, CTX->N);
	GOSTHashAdd512(CTX->Sigma, data, CTX->Sigma);

}

static void GOSTHashStage_3(TGOSTHashContext* CTX) {

	vect internal;
	memset(internal, 0x00, BLOCK_SIZE);
	// Формируем строку с размером сообщения
	internal[1] = ((CTX->buf_size * 8) >> 8) & 0xff;
	internal[0] = (CTX->buf_size * 8) & 0xff;

	GOSTHashPadding(CTX); // Дополняем оставшуюся часть до полных
						  // 64 байт

	GOSTHashG(CTX->h, CTX->N, (const uint8_t*)&(CTX->buffer));

	// Формируем контрольную сумму сообщения
	GOSTHashAdd512(CTX->N, internal, CTX->N);
	GOSTHashAdd512(CTX->Sigma, CTX->buffer, CTX->Sigma);

	GOSTHashG(CTX->h, CTX->v_0, (const uint8_t*)&(CTX->N));
	GOSTHashG(CTX->h, CTX->v_0, (const uint8_t*)&(CTX->Sigma));

	memcpy(CTX->hash, CTX->h, BLOCK_SIZE); // Записываем результат
										   // в нужное место
}

void GOSTHashUpdate(TGOSTHashContext* CTX,
	const uint8_t* data,
	size_t len) {

	while ((len > 63) && (CTX->buf_size) == 0)
	{
		GOSTHashStage_2(CTX, data);
		data += 64;
		len -= 64;
	}

	size_t chk_size; // Объем незаполненной части буфера
	while (len)
	{
		chk_size = 64 - CTX->buf_size;
		if (chk_size > len)
			chk_size = len;
		// Дописываем незаполненную часть буфера
		memcpy(&CTX->buffer[CTX->buf_size], data, chk_size);
		CTX->buf_size += chk_size;
		len -= chk_size;
		data += chk_size;
		if (CTX->buf_size == 64)
		{
			// Если буфер заполнился полностью, то
			// делаем еще один второй этап
			GOSTHashStage_2(CTX, CTX->buffer);
			CTX->buf_size = 0;
		}
	}

}

void GOSTHashFinal(TGOSTHashContext* CTX) {

	// Запускаем третий этап
	GOSTHashStage_3(CTX);
	CTX->buf_size = 0;

}

static void reverse_C() {

	unsigned char tmp;

	for (int i = 0; i < 12; i++) {

		for (int j = 0; j < 64/2; j++) {
			tmp = C[i][j];
			C[i][j] = C[i][63 - j];
			C[i][63 - j] = tmp;
		}
	}

}