#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#define HAVE_STRUCT_TIMESPEC

#include<stb_image.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

typedef struct {
	int count;
	int argc;
	char* argv;
}point_t;

int* image_processing(void* point);

int main(int argc, char* argv[]) {
	pthread_t* handle;
	point_t* p;



	handle = (pthread_t*)malloc((argc - 1) * sizeof(pthread_t));
	p = (point_t*)malloc((argc - 1) * sizeof(point_t));

	if (handle == NULL) {
		return -1;
	}
	if (p == NULL) {
		return -2;
	}

	for (int i = 1; i < argc; i++) {
		p[i - 1].count = i;
		p[i - 1].argc = argc;
		p[i - 1].argv = argv[i];
		pthread_create(&handle[i - 1], NULL, image_processing, &p[i - 1]);
	}

}

int* image_processing(void* point) {
	point_t* data = (point_t*)point;

	unsigned char* pixel = NULL;
	unsigned char* pixel_re = NULL;
	int width = 0, height = 0, bpp = 0;
	int re_width = 1280, re_height = 720;
	int idx = 0;

	pixel = stbi_load(data->argv, &width, &height, &bpp, 4);



	pixel_re = (unsigned char*)malloc((size_t)width * height * 4 * sizeof(unsigned char));

	if (pixel_re == NULL) {
		return -3;
	}






	
	stbi_image_free(pixel);
	free(pixel_re);


}


