#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdint.h>

#define MEM_SIZE 1024
#define _Get_Pointer(base, offset) ((uint8_t*)base) + (offset)
__inline void _Read_Memory() {

}

#pragma pack(push, 2)
typedef enum {
	FREE = 0,
	SIZE_FLAG = 1
} flags;
#pragma pack(pop)

typedef enum {
	START = 0, 
	MIDDLE = 1, 
	END = 2
}Memory_Positions;

#pragma pack(push, 2)
typedef union {
	uint16_t reference;
	struct {
		unsigned int FREE : 1;
		unsigned int SIZE : 10;
		unsigned int UNUSED : 5;
	} bits;
} manifest;
#pragma pack(pop)

#define FREE_MASK    (1U << 0)        
#define SIZE_MASK    (0x3FFU << 1)    

__inline int MASK(int flag, uint16_t reference) {
	switch (flag)
	{
	case FREE:
		return (reference & FREE_MASK) >> 0;
	case SIZE_FLAG:  
		return (reference & SIZE_MASK) >> 1;
	default:
		return 0;
	}
}


#define _Craft_Manifest(manifest_ptr, size) do { \
	(manifest_ptr)->bits.FREE = 1;				 \
	if ((size) >= (MEM_SIZE)) {					 \
		printf("error\n");						 \
	} else {									 \
		(manifest_ptr)->bits.SIZE = (size);		 \
	}											 \
} while (0)

#define _Update_Memory_Line(count, size) do {	 \
	if ((size) <= 0) {							 \
		printf("error\n");						 \
	} else {									 \
		(count) += (size);						 \
	}											 \
} while(0)

#define _Append_Data_Memory(dst, value, type, pos) do { \
    *((type*)((uint8_t*)(dst) + (pos))) = (value);      \
} while(0)

Heap_Start(void** array) {
	if ((array)) {
		uint16_t pos = 1, size = MEM_SIZE, part_size = 0;

		while (pos < 3 && array[pos] == NULL) {
			size -= sizeof(manifest);
			part_size = size / pos;
			pos++;
		}

		void* mem_block = VirtualAlloc(NULL, part_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!mem_block) {
			printf("error\n");
			return NULL;
		}

		uint16_t temp[3] = {0};

		for (size_t i = START; i <= END; i++) {
			manifest* manifest_ptr = (manifest*)malloc(sizeof(manifest));
			if (manifest_ptr) {
				memset(manifest_ptr, 0, sizeof(manifest));
			}
			else {
				printf("error\n");
				return NULL;
			}

			_Craft_Manifest(manifest_ptr, part_size);

			if (manifest_ptr->reference == 0) {
				printf("error\n");
				return NULL;
			}

			if (temp[2] == 0 && temp[i] == 0) {
				temp[i] = manifest_ptr->reference;
			}
			else { "error"; return NULL;  };

			if (array[i] == 0) {
				switch (i)
				{
				case START:
					_Append_Data_Memory(mem_block, temp[i], uint16_t, START);
					array[i] = _Get_Pointer(mem_block, 0);
					break;

				case MIDDLE:
					_Append_Data_Memory(mem_block, temp[i], uint16_t, part_size);
					array[i] = _Get_Pointer(mem_block, part_size);
					break;

				case END:
					_Append_Data_Memory(mem_block, temp[i], uint16_t, (part_size * 2));
					array[i] = _Get_Pointer(mem_block, (part_size * 2));
					break;

				default:
					break;
				}
			}
		}
	}
	else {
		printf("error\n");
		return NULL;
	}
}

__declspec(deprecated) void* alloc_mem(void* mem_pool) {
	return NULL;
}

int main() {
	void* array[3] = { 0 };
	Heap_Start(array);

	printf("Heap Free List: \n");
	for (size_t i = 0; i < 3 && array[i] != 0; i++) {
		int size = MASK(SIZE_FLAG, *((uint16_t*)array[i])), free = MASK(FREE, *((uint16_t*)array[i]));

		printf("%x starting reference | %p starting address\n", *((uint16_t*)array[i]), array[i]);
		printf("size: %d | FREE: %d \n\n", size, free);
	}
	return 0;
}
