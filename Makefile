#!/bin/bash/

build: tinygram

tinygram: tinygram.c
	mpicc tinygram.c -o tinygram -lm

serial-bw: tinygram
	mpirun -np 1 ./tinygram lenna_bw.pgm serial_lenna_bw_smooth.pgm smooth
	mpirun -np 1 ./tinygram lenna_bw.pgm serial_lenna_bw_blur.pgm blur
	mpirun -np 1 ./tinygram lenna_bw.pgm serial_lenna_bw_mean.pgm mean
	mpirun -np 1 ./tinygram lenna_bw.pgm serial_lenna_bw_emboss.pgm emboss
	mpirun -np 1 ./tinygram lenna_bw.pgm serial_lenna_bw_sharpen.pgm sharpen

parallel-bw: tinygram
	mpirun -np 4 ./tinygram lenna_bw.pgm parallel_lenna_bw_smooth.pgm smooth
	mpirun -np 4 ./tinygram lenna_bw.pgm parallel_lenna_bw_blur.pgm blur
	mpirun -np 4 ./tinygram lenna_bw.pgm parallel_lenna_bw_mean.pgm mean
	mpirun -np 4 ./tinygram lenna_bw.pgm parallel_lenna_bw_emboss.pgm emboss
	mpirun -np 4 ./tinygram lenna_bw.pgm parallel_lenna_bw_sharpen.pgm sharpen

serial-color: tinygram
	mpirun -np 1 ./tinygram lenna_color.pnm serial_lenna_color_smooth.pnm smooth
	mpirun -np 1 ./tinygram lenna_color.pnm serial_lenna_color_blur.pnm blur
	mpirun -np 1 ./tinygram lenna_color.pnm serial_lenna_color_mean.pnm mean
	mpirun -np 1 ./tinygram lenna_color.pnm serial_lenna_color_emboss.pnm emboss
	mpirun -np 1 ./tinygram lenna_color.pnm serial_lenna_color_sharpen.pnm sharpen

parallel-color: tinygram
	mpirun -np 1 ./tinygram lenna_color.pnm parallel_lenna_color_smooth.pnm smooth
	mpirun -np 1 ./tinygram lenna_color.pnm parallel_lenna_color_blur.pnm blur
	mpirun -np 1 ./tinygram lenna_color.pnm parallel_lenna_color_mean.pnm mean
	mpirun -np 1 ./tinygram lenna_color.pnm parallel_lenna_color_emboss.pnm emboss
	mpirun -np 1 ./tinygram lenna_color.pnm parallel_lenna_color_sharpen.pnm sharpen

clean:
	rm -f tinygram serial_* parallel_*
