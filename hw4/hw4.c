#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS		(int)480
#define COLUMNS		(int)640

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

int main( int argc, char **argv )
{

	int				i, j, k, threshold[3], max[3], x, y, s, tmpXimage[ROWS][COLUMNS], tmpYimage[ROWS][COLUMNS], tmpSGMimage[ROWS][COLUMNS];
	FILE			*fp;
	unsigned char	image[ROWS][COLUMNS], ximage[ROWS][COLUMNS], yimage[ROWS][COLUMNS], SGMimage[ROWS][COLUMNS], bimage[ROWS][COLUMNS], head[32];
	char			filename[6][50], ifilename[50], ch;

	strcpy( filename[0], "image1" );
	strcpy( filename[1], "image2" );
	strcpy( filename[2], "image3" );
	header ( ROWS, COLUMNS, head );

	printf( "Maximum of Gx, Gy, SGM\n" );

	// chose the threshold based on the SGM images
	threshold[0] = 50;
	threshold[1] = 20;
	threshold[2] = 10;

	for ( k = 0; k < 3; k ++)
	{
		clear( ximage );
		clear( yimage );

		/* Read in the image */
		strcpy( ifilename, filename[k] );
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

		max[0] = 0; //maximum of Gx
		max[1] = 0; //maximum of Gy
		max[2] = 0; //maximum of SGM

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
				// compare to find the max for Gx
				if (tmpXimage[x][y] > max[0])
					max[0] = tmpXimage[x][y];

				// calcualate Gy using Sobel Operator
				tmpYimage[x][y] = abs(-1*image[x-1][y-1] + 1*image[x+1][y-1]
									+ -2*image[x-1][y] + 2*image[x+1][y]
									+ -1*image[x-1][y+1] + 1*image[x+1][y+1]);
				// compare to find the max for Gy
				if (tmpYimage[x][y] > max[1])
					max[1] = tmpYimage[x][y];
			}
		}

		for (x = 0; x < ROWS; x++) {
			for (y = 0; y < COLUMNS; y++) {

				// Normalize brightness to 255 for Gx and Gy images
				ximage[x][y] = ((float)tmpXimage[x][y]/max[0]) * 255;
				yimage[x][y] = ((float)tmpYimage[x][y]/max[1]) * 255;

				// calcualate SGM using Sobel Operator
				tmpSGMimage[x][y] = tmpXimage[x][y]*tmpXimage[x][y] + tmpYimage[x][y]*tmpYimage[x][y];
				// compare to find the max for SGM
				if (tmpSGMimage[x][y] > max[2])
					max[2] = tmpSGMimage[x][y];
			}
		}

		for (x = 0; x < ROWS; x++) {
			for (y = 0; y < COLUMNS; y++) {

				// Normalize brightness to 255 for SGM
				SGMimage[x][y] = ((float)tmpSGMimage[x][y]/max[2]) * 255;
				
				// Generating the binary image based on the SGM image
				if(threshold[k] > SGMimage[x][y])
					bimage[x][y] = 0;
				if(threshold[k] <= SGMimage[x][y])
					bimage[x][y] = 255;	
			}
		}

		/* Write gray level image */
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, ".ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for(i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);		
		fclose( fp );

		/* Write Gx to a new image*/
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-x.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for(i = 0; i < ROWS; i++) fwrite(ximage[i], 1, COLUMNS, fp);
	    fclose( fp );
					
		/* Write Gy to a new image */
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-y.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for(i = 0; i < ROWS; i++) fwrite(yimage[i], 1, COLUMNS, fp);
		fclose( fp );

		/* Write SGM to a new image */
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-s.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for(i = 0; i < ROWS; i++) fwrite(SGMimage[i], 1, COLUMNS, fp);
		fclose( fp );
		
		/* Write the binary image to a new image */
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-b.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for(i = 0; i < ROWS; i++) fwrite(bimage[i], 1, COLUMNS, fp);
		fclose( fp );

		printf( "%d %d %d\n", max[0], max[1], max[2] );

	}

	// Commented out, not needed
	// printf( "Press any key to exit: " );
	// gets( &ch );
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
