#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "Filters.h"

int getMarginSize(int Size);
int getNorm(int Size_Gauss, int gauss_mask[]);
int validateRGB(int Norm, int RGBParam);

int main(int argc, char** argv) {
	/*
	////////////////////////////////////// gausian big mask
	int sz = 10;//musi byc nieparzyste ale czemu to nie wiem jak nizej jest warunekt który to zapewnia.
	int MatrixSize = sz * 2 + 1;
	double sigma = (sz * 2 + 1) / 3;
	int gausianMask[MatrixSize][MatrixSize];
	int gausianMaskVec[MatrixSize*MatrixSize];
	int x0 = sz;
	int y0 = sz;
	double coeff = 1 / (2 * M_PI * pow(sigma, 2));
	double w = 2 * pow(sigma, 2);

	for (int i = 0; i < MatrixSize; i++)
	{
		for (int j = 0; j < MatrixSize; j++) {
			int a = i - x0;
			int b = j - y0;
			//gausianMask[i][j] = coeff * exp(−(pow(a, 2) + pow(b, 2)) / w);
			double ee = exp((-(pow(a, 2) + pow(b, 2)) / w));
			gausianMask[i][j] = (int)(coeff * ee * 100000);
			//printf(" res: %d, a=%f, b=%f, coeff=%f, w=%f ,ee=%f \n ", gausianMask[i][j], a, b, coeff, w, ee);
			printf(" %d ", gausianMask[i][j]);
			gausianMaskVec[MatrixSize*i + j] = gausianMask[i][j];
		}
		printf("\n");
	}
	//////////////////////////////////////////////////////////////
	*/

	printf("Sequential Gauss Algorithm \n\n");
	//char* filename = "jaws.bmp";
	//char [70] namebuffer;

	char* inputfilename = "4k.bmp";
	char* outputfilename = "output.bmp";
	printf("inputfile = ");
	printf( inputfilename);

	static unsigned char *texels;
	static unsigned char *texelsout;
	static int width, height;
	FILE *fd;


	fd = fopen(inputfilename, "rb");
	if (fd == NULL)
	{
		printf("Error: fopen failed\n");
		return 1;
	}

	unsigned char header[54];

	// Read header
	fread(header, sizeof(unsigned char), 54, fd);

	// Capture dimensions
	width = *(int*)&header[18];
	height = *(int*)&header[22];
	printf("\nWidth = %i \nHeight = %i\n", width, height);
	int padding = 0;

	// Calculate padding
	while ((width * 3 + padding) % 4 != 0)
	{
		padding++;
	}

	// Compute new width, which includes padding
	int widthnew = width * 3 + padding;

	// Allocate memory to store image data (non-padded)
	texels = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
	texelsout = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
	if (texels == NULL)
	{
		printf("Error: Malloc failed. when you try to allocate bmp memory\n");
		return 1;
	}

	// Allocate temporary memory to read widthnew size of data
	unsigned char* data = (unsigned char *)malloc(widthnew * sizeof(unsigned int));

	// Read row by row of data and remove padded data.
	for (int i = 0; i < height; i++)
	{
		// Read widthnew length of data
		fread(data, sizeof(unsigned char), widthnew, fd);

		// Retain width length of data, and swizzle RB component.
		// BMP stores in BGR format, my usecase needs RGB format
		for (int j = 0; j < width * 3; j += 3)
		{
			int index = (i * width * 3) + (j);
			texels[index + 0] = data[j + 0];//r -> B
			texels[index + 1] = data[j + 1];//g -> G
			texels[index + 2] = data[j + 2];//b -> R
		}
	}

	free(data);





	/*int Size = GAUSS5_SIZE;
	int Norm = getNorm(GAUSS5_SIZE, GAUSS5);
	int margin = getMarginSize(GAUSS5_SIZE);
	*/
	int Size = GAUSS21_SIZE;
	int Norm = getNorm(Size, GAUSS21);
	int margin = getMarginSize(Size);

	printf("\n============================\n");
	printf("Margin = %i \n", margin);
	printf("wielkosc  = %i \n", Size);
	printf("suma w gausie w masce  = %i \n", Norm);
	printf("============================\n");

	int rsume, gsume, bsume;

	for (int i = margin; i < height - margin; i++)
		for (int j = margin * 3; j < 3 * (width - margin); j += 3) {
			rsume = 0;
			gsume = 0;
			bsume = 0;
			for (int k = 0; k < Size; k++) {
				for (int l = 0; l < Size; l++) {
					//int index1 = (i * width * 3) + (j);
					int index = ((i + k - Size / 2) * width * 3) + (j + ((l - Size / 2) * 3));
					rsume += GAUSS21[k*Size + l] * texels[index + 2];
					gsume += GAUSS21[k*Size + l] * texels[index + 1];
					bsume += GAUSS21[k*Size + l] * texels[index + 0];
				}
			}
			/*
			rsume /= Norm;
			gsume /= Norm;
			bsume /= Norm;
			if (rsume > 255) rsume = 255;
			else if (rsume < 0) rsume = 0;
			if (gsume > 255) gsume = 255;
			else if (gsume < 0) gsume = 0;
			if (bsume > 255) bsume = 255;
			else if (bsume < 0) bsume = 0;
			*/
			rsume = validateRGB(Norm, rsume);
			gsume = validateRGB(Norm, gsume);
			bsume = validateRGB(Norm, bsume);


			int index1 = (i * width * 3) + (j);
			texelsout[index1 + 0] = bsume; //rsume;
			texelsout[index1 + 1] = gsume;
			texelsout[index1 + 2] = rsume; //bsume;

		}
	//fclose(fd);

	FILE *fo;
	fo = fopen(outputfilename, "w");
	fwrite(header, sizeof(unsigned char), 54 * sizeof(unsigned char), fo);
	fwrite(texelsout, sizeof(unsigned char), width * height * 3 * sizeof(unsigned char), fo);

	fclose(fo);
	fclose(fd);
	free(texels);
	free(texelsout);
}
/*int **generateGausianMatrix(int MatrixSize, double sigma) {
	int[MatrixSize][MatrixSize] gausianMask;

	int x0 = 64;
	int y0 = 64;
	double w = ;
	for (int i = 0; i < MatrixSize; i++)
	{
		for (int j = 0; j < MatrixSize; j++) {
			int a = i - x0;
			int b = j - y0;
			double coeff = 1 / (2 * M_PI*pow(sigma, 2));
			double w = 2 * pow(sigma, 2);
			gausianMask[i][j] = coeff*exp(−(pow(a, 2) + pow(b, 2)) / w);
		}
	}

}*/

int getMarginSize(int Size) {
	int margin = ((Size - 1) / 2);
	return margin;
}
int getNorm(int Size_Gauss, int gauss_mask[]) {
	int Norm = 0;
	for (int i = 0; i < Size_Gauss; i++)
		for (int j = 0; j < Size_Gauss; j++)
			Norm += gauss_mask[i + Size_Gauss*j];
	if (Norm == 0) Norm = 1;
	return Norm;
}
int validateRGB(int Norm, int RGBParam) {
	RGBParam /= Norm;
	if (RGBParam > 255) RGBParam = 255;
	else if (RGBParam < 0) RGBParam = 0;
	return RGBParam;
}


/*int main(int argc, char *argv[]) {
printf("Hello World from sequential Gauss Attemption \n");
//unsigned char *texels;
//texels = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
// texels = (unsigned char *) malloc(500 * 500 * 3 * sizeof(unsigned char));
//unsigned char texels = readBmp("jaws.bmp");
readBmp("jaws.bmp");
//printf("texels = %d", texels);

}*/
/*int asd() {


	int rsume, gsume, bsume, graysume;
	int margin = ((Size - 1) / 2);

	for (int i = margin; i < ObrazKolorowy->Width - margin; i++)
		for (int j = margin; j < ObrazKolorowy->Height - margin; j++) {
			rsume = 0;
			gsume = 0;
			bsume = 0;
			for (int k = 0; k < Size; k++)
				for (int l = 0; l < Size; l++) {
					rsume += Filter[k*Size + l] * red[i + k - margin][j + l - margin];
					gsume += Filter[k*Size + l] * green[i + k - margin][j + l - margin];
					bsume += Filter[k*Size + l] * blue[i + k - margin][j + l - margin];
				}
			rsume /= Norm;
			gsume /= Norm;
			bsume /= Norm;

			if (rsume > 255) rsume = 255;
			else if (rsume < 0) rsume = 0;
			if (gsume > 255) gsume = 255;
			else if (gsume < 0) gsume = 0;
			if (bsume > 255) bsume = 255;
			else if (bsume < 0) bsume = 0;

			WynikKolorowy->Canvas->Pixels[i][j] = (TColor)rsume + (gsume << 8) + (bsume << 16);
		}
}*/

/*void readBmp(char *inputfilename)
{
	static unsigned char *texels;
	static int width, height;
	FILE *fd;

	fd = fopen(inputfilename, "rb");
	if (fd == NULL)
	{
		printf("Error: fopen failed\n");
		return;
	}

	unsigned char header[54];

	// Read header
	fread(header, sizeof(unsigned char), 54, fd);

	// Capture dimensions
	width = *(int*)&header[18];
	height = *(int*)&header[22];
	printf("Width = %i \nHeight = %i\n", width, height);
	int padding = 0;

	// Calculate padding
	while ((width * 3 + padding) % 4 != 0)
	{
		padding++;
	}

	// Compute new width, which includes padding
	int widthnew = width * 3 + padding;

	// Allocate memory to store image data (non-padded)
	texels = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
	if (texels == NULL)
	{
		printf("Error: Malloc failed\n");
		return;
	}

	// Allocate temporary memory to read widthnew size of data
	unsigned char* data = (unsigned char *)malloc(widthnew * sizeof(unsigned int));

	// Read row by row of data and remove padded data.
	for (int i = 0; i < height; i++)
	{
		// Read widthnew length of data
		fread(data, sizeof(unsigned char), widthnew, fd);

		// Retain width length of data, and swizzle RB component.
		// BMP stores in BGR format, my usecase needs RGB format
		for (int j = 0; j < width * 3; j += 3)
		{
			int index = (i * width * 3) + (j);
			texels[index + 0] = data[j + 2];
			texels[index + 1] = data[j + 1];
			texels[index + 2] = data[j + 0];
		}
	}

	free(data);
	fclose(fd);
}*/