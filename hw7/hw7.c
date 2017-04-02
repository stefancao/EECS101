#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROWS		(int)480
#define COLUMNS		(int)640
#define X_ORIGIN 	(int)ROWS/2
#define Y_ORIGIN 	(int)COLUMNS/2
#define VECTOR_SIZE	(int)3

double dotProduct(double x[3], double y[3]);
void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

int main( int argc, char **argv )
{
	int				i, j, k; 
	FILE			*fp;
	unsigned char	image[ROWS][COLUMNS], head[32];
	char			filename[9][50];
	double 			L[ROWS][COLUMNS];	// scene radiance

	double 			H[3]; 		// angular bisector between V and S
	double 			N[3];		// surface normal to the sphere
	double 			V[3] = {0,0,1};		// viewing direction (always (0,0,1))

	// source direction
	double 			S[9][3] = { {0,0,1}, {1/sqrt(3),1/sqrt(3),1/sqrt(3)}, 
								{1,0,0}, {0,0,1}, {0,0,1}, {0,0,1}, {0,0,1},
								{0,0,1}, {0,0,1} };
	// radius
	double 			r[9] = { 50, 50, 50, 10, 100, 50, 50, 50, 50 };

	// a - constant between 0 and 1
	double 			a[9] = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.1, 1, 0.5, 0.5 };

	// m - constant that is related to the roughness of the surface and alpha
	double 			m[9] = { 1, 1, 1, 1, 1, 1, 1, 0.1, 10000 };

	strcpy(filename[0], "image1");
	strcpy(filename[1], "image2");
	strcpy(filename[2], "image3");
	strcpy(filename[3], "image4");
	strcpy(filename[4], "image5");
	strcpy(filename[5], "image6");
	strcpy(filename[6], "image7");
	strcpy(filename[7], "image8");
	strcpy(filename[8], "image9");

	header ( ROWS, COLUMNS, head );

	// generating 9 different images with different params
	for (k = 0; k < 9; k++) {

		// finding angluar bisector
		for (i = 0; i < VECTOR_SIZE; i++)
			H[i] = (V[i] + S[k][i]) / sqrt((pow(V[0]+S[k][0], 2) + pow(V[1]+S[k][1], 2) + pow(V[2]+S[k][2], 2)));

		for (i = 0; i < ROWS; i++)
			for (j = 0; j < COLUMNS; j++) {

				int x = i - X_ORIGIN;
				int y = j - Y_ORIGIN;

				// finding the surface normal to the sphere
				double p = -x / sqrt( pow(r[k],2) - pow(x,2) - pow(y,2) );
				double q = -y / sqrt( pow(r[k],2) - pow(x,2) - pow(y,2) );
				N[0] = p / sqrt( pow(p,2) + pow(q,2) + 1 );
				N[1] = q / sqrt( pow(p,2) + pow(q,2) + 1 );
				N[2] = 1 / sqrt( pow(p,2) + pow(q,2) + 1 );

				// Lambertian reflectance; L_l = cos(theta) = a dot b
				double L_l = dotProduct(N, S[k]);

				// angle between the surface normal N and H
				double alpha = acos(dotProduct(N,H));

				// Specular reflectance
				double L_s = exp( - pow(alpha/m[k], 2) );

				// Scene radiance
				L[i][j] = a[k]*L_l + (1-a[k])*L_s;

			}

		// finding max to normalize
		double max = 0.0;
		for (i = 0; i < ROWS; i++)
			for (j = 0; j < COLUMNS; j++) {
				if (L[i][j] > max)
					max = L[i][j];
			}

		// normalize
		for (i = 0; i < ROWS; i++)
			for (j = 0; j < COLUMNS; j++)
				image[i][j] = (float) L[i][j] / max * 255;
			
		// writing the images to file
		if (!( fp = fopen( strcat( filename[k], ".ras" ), "wb" ) )) {
			fprintf( stderr, "error: could not open %s\n", filename[k] );
			exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for(i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose( fp );	
	}	

	return 0;
}

double dotProduct(double x[3], double y[3]) {
	double result = 0.0;
	int i;
	for(i = 0; i < VECTOR_SIZE; i++)
		result += x[i] * y[i];
	return result;
}

void clear( unsigned char image[][COLUMNS] )
{
	int	i,j;
	for ( i = 0 ; i < ROWS ; i++ )
		for ( j = 0 ; j < COLUMNS ; j++ ) image[i][j] = 0;
}

void header( int row, int col, unsigned char head[32] )
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */
	
	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch ++; 
	head[6] = *ch;
	ch ++;
	head[5] = *ch;
	ch ++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch ++; 
	head[10] = *ch;
	ch ++;
	head[9] = *ch;
	ch ++;
	head[8] = *ch;
	
	ch = (char*)&num;
	head[19] = *ch;
	ch ++; 
	head[18] = *ch;
	ch ++;
	head[17] = *ch;
	ch ++;
	head[16] = *ch;
	

	/* Big-endian for unix */
	/*
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
*/
}
