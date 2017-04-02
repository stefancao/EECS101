#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROWS		(int)480
#define COLUMNS		(int)640
#define VOTINGRHO 	(int)COLUMNS*2
#define VOTINGTHETA (int)180

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

int main( int argc, char **argv )
{

	int				i, j, k, max, x, y, s, sgm_threshold, hough_threshold;
	int				tmpXimage[ROWS][COLUMNS], tmpYimage[ROWS][COLUMNS], tmpSGMimage[ROWS][COLUMNS], voting[VOTINGTHETA][VOTINGRHO];
	FILE			*fp;
	unsigned char	image[ROWS][COLUMNS], head[32];
	char			filename[50], ifilename[50], ch;

	strcpy( filename, "image" );
	header ( ROWS, COLUMNS, head );

	clear( image );

	/* Read in the image */
	strcpy( ifilename, filename );
	if (( fp = fopen( strcat(ifilename, ".raw"), "rb" )) == NULL )
	{
	  fprintf( stderr, "error: couldn't open %s\n", ifilename );
	  exit( 1 );
	}			
	for ( i = 0; i < ROWS ; i++ )
	  if ( fread( image[i], sizeof(char), COLUMNS, fp ) != COLUMNS )
	  {
		fprintf( stderr, "error: couldn't read enough stuff\n" );
		exit( 1 );
	  }
	fclose( fp );

	max = 0;
	sgm_threshold = 100;

	/* Compute Gx, Gy, SGM, find out the maximum and normalize*/
	for (x = 0; x < ROWS; x++) {
		for (y = 0; y < COLUMNS; y++) {
			
			// skip the borders/ set them to 0
			if (x == 0 || x == ROWS-1 || y == 0 || y == COLUMNS-1) {
				tmpXimage[x][y] = 0;
				tmpYimage[x][y] = 0;
				continue;
			}
			
			// calcualate Gx using Sobel Operator
			tmpXimage[x][y] = abs(-1*image[x-1][y-1] + -2*image[x][y-1] + -1*image[x+1][y-1]
								+ 1*image[x-1][y+1] + 2*image[x][y+1] + 1*image[x+1][y+1]);
			
			// calcualate Gy using Sobel Operator
			tmpYimage[x][y] = abs(-1*image[x-1][y-1] + 1*image[x+1][y-1]
								+ -2*image[x-1][y] + 2*image[x+1][y]
								+ -1*image[x-1][y+1] + 1*image[x+1][y+1]);

			// calcualate SGM using Sobel Operator
			tmpSGMimage[x][y] = tmpXimage[x][y]*tmpXimage[x][y] + tmpYimage[x][y]*tmpYimage[x][y];
			// compare to find the max for SGM
			if (tmpSGMimage[x][y] > max)
				max = tmpSGMimage[x][y];
		}
	}

	// generate binary image based on SGM image
	for (x = 0; x < ROWS; x++) {
		for (y = 0; y < COLUMNS; y++) {

			// Normalize brightness to 255 for SGM
			image[x][y] = ((float)tmpSGMimage[x][y]/max) * 255;
			
			// Generating the binary image based on the SGM image
			if(sgm_threshold > image[x][y])
				image[x][y] = 0;
			if(sgm_threshold <= image[x][y])
				image[x][y] = 255;	
		}
	}

	/* Write binary image */
	strcpy( ifilename, filename );
	if (!( fp = fopen( strcat( ifilename, "-binary.ras" ), "wb" ) ))
	{
	  fprintf( stderr, "error: could not open %s\n", ifilename );
	  exit( 1 );
	}
	fwrite( head, 4, 8, fp );
	for(i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);		
	fclose( fp );		

	// init voting array to 0
	// voting array was chose as voting[180][COLUMNS*2], explanation in the report
	for (x = 0; x < VOTINGTHETA; x++) {
		for (y = 0; y < VOTINGRHO; y++) {
			voting[x][y] = 0;
		}
	}

	// the max rho can be is the same size as number of columns
	int maxRho = COLUMNS;

	// calculate rho and theta
	for (x = 0; x < ROWS; x++) {
		for (y = 0; y < COLUMNS; y++) {
			if (image[x][y] == 255) {
				int tmpTheta;
				for (tmpTheta = 0; tmpTheta < 180; tmpTheta++) {
					double radian = ((double)3.14/180) * tmpTheta;
					double rho = y * cos(radian) - x * sin(radian);
					voting[tmpTheta][(int)rho + maxRho]++;
				}
			}
		}
	}

	int max1 = 0;
	int max2 = 0;
	int max3 = 0;
	int theta1 = 0; 
	int theta2 = 0;
	int theta3 = 0;
	int rho1 = 0;
	int rho2 = 0;
	int rho3 = 0;

	// chosen to have three different lines
	hough_threshold = 20;

	// find the first line
	for (x = 0; x < VOTINGTHETA; x++) {
		for (y = 0; y < VOTINGRHO; y++) {
			if(max1 < voting[x][y]) {
				max1 = voting[x][y];
				theta1 = x;
				rho1 = y;
			}
		}
	}

	// set what was found to be 0 so we don't use the same line again
	voting[theta1][rho1] = 0;

	// find the second line
	for (x = 0; x < VOTINGTHETA; x++) {
		for (y = 0; y < VOTINGRHO; y++) {
			if(max2 < voting[x][y]) {

				// make sure that the lines are not too close to the line already found
				if(voting[x][y] > (max1-hough_threshold) && voting[x][y] < (max1+hough_threshold))
					continue;
				max2 = voting[x][y];
				theta2 = x;
				rho2 = y;
			}
		}
	}

	// set what was found to be 0 so we don't use the same line again
	voting[theta2][rho2] = 0;

	// find the third line
	for (x = 0; x < VOTINGTHETA; x++) {
		for (y = 0; y < VOTINGRHO; y++) {
			if(max3 < voting[x][y]) {

				// make sure that the lines are not too close to the line already found
				if(voting[x][y] > (max2-hough_threshold) && voting[x][y] < (max2+hough_threshold))
					continue;
				max3 = voting[x][y];
				theta3 = x;
				rho3 = y;
			}
		}
	}

	rho1 -= maxRho;
	rho2 -= maxRho;
	rho3 -= maxRho;

	// print out the values found
	printf("rho  :%d\t%d\t%d\n", rho1, rho2, rho3);
	printf("theta:%d\t%d\t%d\n", theta1, theta2, theta3);
	printf("max  :%d\t%d\t%d\n", max1, max2, max3);
	
	// since we added maxRho before (so that it would fit in the array), 
	// 		we need to subtract it again to find the real values
	
	// convert to radians
	double radian1 = (double)3.14/180 * theta1;
	double radian2 = (double)3.14/180 * theta2;
	double radian3 = (double)3.14/180 * theta3;

	// loop through and if it satisfies the line, set that pixel to white
	for(x = 0; x < ROWS; x++) {		
		y = (float)x*tan(radian1) + rho1/cos(radian1);
		if(y < COLUMNS && y >= 0)
			image[x][y] = 255;
		y = (float)x*tan(radian2) + rho2/cos(radian2);
		if(y < COLUMNS && y >= 0)
			image[x][y] = 255;
		y = (float)x*tan(radian3) + rho3/cos(radian3);
		if(y < COLUMNS && y >= 0)
			image[x][y] = 255;
	}

	/* Write line edge image */
	strcpy( ifilename, filename );
	if (!( fp = fopen( strcat( ifilename, "-lineedge.ras" ), "wb" ) ))
	{
	  fprintf( stderr, "error: could not open %s\n", ifilename );
	  exit( 1 );
	}
	fwrite( head, 4, 8, fp );
	for(i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);		
	fclose( fp );	

	return 0;
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
