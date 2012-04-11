//============================================================================
// Name        : ReverseTimeMigration.cpp
// Author      : Makoto Sadahiro
// Version     : version will be indicated in execute message
// Copyright   : Texas Advanced Computing Center
// Description : Reverse Time Migration for 2-D (XT to XZ) space
//============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <limits>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>	// setting precision of output
//#include <omp.h>

using namespace std;

const string rtm_build_date = __DATE__;
const string rtm_build_time = __TIME__;


bool longdouble2pgm(string szFilename, long double* s_plane, int dim_x,
    int dim_z){

	cout << endl << "longdouble2pgm " << szFilename << " begin" << endl;

	// allocate local memory
	long double* d_plane = new long double[dim_x * dim_z];

	// read in from source
	for(int i = 0; i < dim_x * dim_z; i++){
		d_plane[i] = s_plane[i];
}

	// opening files for output
	szFilename += ".pgm";
	ofstream outfile;
	outfile.open(szFilename.c_str());
	if(!outfile){
		cerr << "file not found" << endl;
		return false;
	}

	// need conversion from double to int(0-255) here -> 65536
	long double d_max = - numeric_limits<long double>::max();
	long double d_min = numeric_limits<long double>::max();
	long double d_nmin = - numeric_limits<long double>::max();
	long double d_nmax = numeric_limits<long double>::max();

//	cout << "test" << " " << d_nmax << " " << d_nmin << " " << d_min << " " << d_max << endl;

	int max_idx, min_idx, nmin_idx, nmax_idx;
	max_idx = min_idx = nmin_idx = nmax_idx = 0;

	for(int i = 0; i < dim_x * dim_z; i++){
		if(d_plane[i] > d_max){
			d_max = d_plane[i];
			max_idx = i;
		}
		if(d_plane[i] < d_nmax){
			d_nmax = d_plane[i];
			nmax_idx = i;
		}
		if((d_plane[i] > d_nmin) && (d_plane[i] < 0.0)){
			d_nmin = d_plane[i];
			nmin_idx = i;
		}
		if((d_plane[i] < d_min) && (d_plane[i] > 0.0)){
			d_min = d_plane[i];
			min_idx = i;
		}
	}
	if(d_nmin == (- numeric_limits<long double>::max())){
		d_nmin = d_min;
		nmin_idx = min_idx;
	}
	if(d_nmax == numeric_limits<long double>::max()){
		d_nmax = d_min;
		nmax_idx = min_idx;
	}
	cout << "source value range: [ " << d_nmax << " .. " << d_nmin <<
			" .. " << d_min << " .. " << d_max << " ]" << endl;


	long double range = d_plane[max_idx] - d_plane[nmax_idx];
	if(range == 0.0){
		range = 1.0;
	}
	if(range < 1.0){
		range = 1.0;
	}
	cout << "new variable range (should be more than 0.1): " << range << endl;

	// converted grid
	long int* i_plane = new long int[dim_x * dim_z];

	int max_pgm_val = 0;

		d_nmax = d_plane[nmax_idx];
		cout << "new min: " << d_nmax << endl;

	long double v0, v1, v2;
	for(int j = 0; j < dim_x * dim_z; j++){

		if(j == nmax_idx){
					cout << "d_nmax: " << d_plane[nmax_idx] << endl;
		}else if(j == min_idx){
					cout << "d_min: " << d_plane[min_idx] << endl;
		}else if(j == max_idx){
					cout << "d_max: " << d_plane[max_idx] << endl;
		}

		v0 = d_plane[j];
		d_plane[j] = d_plane[j] - d_nmax;
		v1 = d_plane[j];
		d_plane[j] = d_plane[j] * 65535.0;
		v2 = d_plane[j];
		d_plane[j] = d_plane[j] / range;
		if(d_plane[j] < 1.0){
			d_plane[j] = 0.0;
		}
		i_plane[j] = (long int)(d_plane[j]);
		if(i_plane[j] > max_pgm_val){
			max_pgm_val = i_plane[j];
		}

		if(i_plane[j] > 65535){
			cerr << v2 << "/" << range << "=" << v2 / range << "?=" << d_plane[j] << endl;
			cerr << "longdouble2pgm error, above 65535: " << endl <<
					"index:" << j <<  " " << v0 << " -> " << v1 << " mul ->  " << v2 << " div ->  " <<
					d_plane[j] << " -> " << i_plane[j] << endl;
			i_plane[j] = 65535;
			getchar();
		}
		if(i_plane[j] < 0){
			cerr << v2 << "/" << range << "=" << v2 / range << "?=" << d_plane[j] << endl;
			cerr << "longdouble2pgm error, below 0: " << endl <<
					"index:" << j <<  " " << v0 << " sub ->  " << v1 << " mul ->  " << v2 << " div ->  " <<
					d_plane[j] << " -> " << i_plane[j] << endl;
			i_plane[j] = 0;
			getchar();
		}
	}

	// draw a frame
//	for(int fx = 0; fx < dim_x; fx++){
//		i_plane[3001*dim_x+fx] = 0;
//		i_plane[(dim_z-3001)*dim_x+fx] = 0;
//	}
//	for(int fz = 3001; fz < dim_z - 3001; fz++){
//		i_plane[fz*dim_x] = 0;
//		i_plane[(fz-3001)*dim_x] = 0;
//	}

	// write out
	outfile << "P2" << endl
			<< "# original is long double" << endl
			<< dim_x << " " << dim_z << endl
	    << max_pgm_val << endl;
	for(int y = 0; y < dim_z; y++){
		for(int x = 0; x < dim_x; x++){
			outfile << i_plane[y * dim_x + x] << endl;
		}
	}

	cout << "finished writing " << szFilename.c_str() << "." << endl;

	outfile.close();

	delete [] i_plane;
	delete [] d_plane;

	return true;
}


long double load_array(string filename, long double* &e_grid, unsigned int x,
    unsigned int z){

	cout << "reading float elements from: " << filename << endl;

	// process file
	FILE *file_in;
	if((file_in = fopen(filename.c_str(), "r")) == NULL){
		cout << "sourceput file \"" << filename << "\" does not exist" << endl
		    << "exiting...." << endl;
		return false;
	}//file opening assuarance

	// load file in transpose (trace to row major)
	float a_trace[z];
	unsigned int num_read = 0;
	long double v_max = 0.0;
	long double v_min = numeric_limits<long double>::max();

	for(unsigned int xc = 0; xc < x; xc++){

		num_read = fread(a_trace, sizeof(float), z, file_in);
		if(num_read != z)
			cout << "error: reading counter in elements" << endl;

		for(unsigned int tc = 0; tc < z; tc++){

			e_grid[tc * x + xc] = (long double)(a_trace[tc]);
			if(e_grid[tc * x + xc] > v_max)
				v_max = e_grid[tc * x + xc];
			if(e_grid[tc * x + xc] < v_min)
				v_min = e_grid[tc * x + xc];

		}
	}

	if(fclose(file_in) == -1){
		cout << "input file did not close correctly" << endl << "exiting...."
		    << endl;
		return 0.0;
	}

	cout << "value range: [ " << v_min << " .. " << v_max << " ]" << endl;

	// write out pgm
	bool is_pgm_wrote;
	is_pgm_wrote = longdouble2pgm(filename, e_grid, x, z);
	if(!is_pgm_wrote)
		cout << "error writing pgm:" << filename << endl;
	cout << endl << "load_array " << filename << " done." << endl;

	long double r_max = abs(v_max);
	long double r_min = abs(v_min);
	if(r_max > r_min)
		return r_max;
	else
		return r_min;
}


//                                                 need & before vel_grid?
long double import_vel(string filename, long double* vel_grid, unsigned int x,
    unsigned int t, long double v_mux, long double dt,
    long double &max_dz, unsigned int & z){

	cout << "loading velocity model" << endl;

	long double ret_value = 1500.0;

	// read in (intermediate) velocity model
	ret_value = load_array(filename, vel_grid, x, t);


	// Multiplier processing
	cout << "velocity multiplier: " << v_mux << endl;
	for(unsigned int v = 0; v < x * t; v++){
		vel_grid[v] *= v_mux;
	}


	// creating depth map for each ms
	long double* xtz_grid = new long double [x * t];
	long double max_int_velocity = 0.0;
	long double max_travel_dist = 0.0;

	// creating z-depth map from t-depth map
	// base (surface) line
	for(unsigned int x_itr = 0; x_itr < x; x_itr++){
//		xtz_grid[x_itr] = vel_grid[x_itr] * 0.001;
		xtz_grid[x_itr] = vel_grid[x_itr] * dt;
		if(vel_grid[x_itr] > max_int_velocity){
			max_int_velocity = vel_grid[x_itr];
		}
		if(xtz_grid[x_itr] > max_travel_dist){
			max_travel_dist = xtz_grid[x_itr];
		}
	}
	// sub-surfaces with value dependency to surface
	for(unsigned int z_itr = x; z_itr < x * t; z_itr++){
//		xtz_grid[z_itr] = xtz_grid[z_itr - x] + (vel_grid[z_itr] * 0.001);
		xtz_grid[z_itr] = xtz_grid[z_itr - x] + (vel_grid[z_itr] * dt);
		if(vel_grid[z_itr] > max_int_velocity){
			max_int_velocity = vel_grid[z_itr];
		}
		if(xtz_grid[z_itr] > max_travel_dist){
			max_travel_dist = xtz_grid[z_itr];
		}
	}

	// we have max int velocity (thus max int travel distance) and global travel distance now
	cout << "global travel distance is: " << max_travel_dist << endl;
//	max_dz = max_int_velocity * 0.001;
	max_dz = max_int_velocity * dt;
	cout << "explicit delta_z (max_dz): " << max_dz << endl;
//	max_dz = ceil(max_dz) * 1.1; //                                 dz adjustment here
	cout << "ceiling delta_z (max_dz): " << max_dz << endl;
//	z = ceil(max_travel_dist / max_dz);
	z = ceil(max_travel_dist / max_dz) + t * 2;  // mem hack for top and bottom padding
	cout << "calculated z-dim with ceiling of delta_z is: " << z << " rows." << endl;
	longdouble2pgm("xtz_grid", xtz_grid, x, t);

	// padding space count for top
	int top_pad = x * t;

	// create and initialize new depth grid and velocity grid by xz
	long double* z_grid = new long double [x * z];
	long double* xzv_grid = new long double[x * z];

	for(unsigned int z_itr = 0; z_itr < t; z_itr++){
		for(unsigned int x_itr = 0; x_itr < x; x_itr++){
			z_grid[z_itr * x + x_itr] = 0.0;
		}
	}
	for(unsigned int z_itr = 0; z_itr < z - t; z_itr++){
		for(unsigned int x_itr = 0; x_itr < x; x_itr++){
			z_grid[z_itr * x + x_itr + top_pad] = z_itr * max_dz;
		}
	}
	for(unsigned int z_itr = 0; z_itr < z; z_itr++){
		for(unsigned int x_itr = 0; x_itr < x; x_itr++){
			xzv_grid[z_itr * x + x_itr] = 1500.0;
		}
	}
	cout << "z_grid and xzv_grid were generated and initialized" << endl;
	longdouble2pgm("z_grid", z_grid, x, z);

	// fill xzv_grid with interpolated velocities from v_grid with ref to xtz_grid
	for(unsigned int x_itr = 0; x_itr < x; x_itr++){
		xzv_grid[x_itr + top_pad] = vel_grid[x_itr];
	}
	// 2nd row and on
	cout << "Velocity interpolation: ";
	for(unsigned int z_itr = 1 + t; z_itr < z; z_itr++){
		for(unsigned int x_itr = 0; x_itr < x; x_itr++){
//		cout << "row " << z_itr << "  col " << x_itr << endl;

			unsigned int xzv_curr = z_itr * x + x_itr;
			long double target_z = z_grid[xzv_curr];

			unsigned int curr_t_idx = 0;
			while((xtz_grid[curr_t_idx * x + x_itr] < target_z) && (curr_t_idx < t)){
				curr_t_idx++;
			}
//			cout << "ct:" << curr_t_idx << "     " <<  xtz_grid[curr_t_idx * x + x_itr] << ">=" << target_z << endl;

			if(curr_t_idx >= t){
				// reached to the bottom.  velocity is the same as 1z above
				xzv_grid[xzv_curr] = xzv_grid[xzv_curr - x];
			}else{
				// linear interpolation of velocity by ratio
				long double prev_z = xtz_grid[(curr_t_idx - 1) * x + x_itr];
				long double curr_z = xtz_grid[curr_t_idx * x + x_itr];
				long double prev_v = vel_grid[(curr_t_idx - 1) * x + x_itr];
				long double curr_v = vel_grid[curr_t_idx * x + x_itr];
				long double aab = (target_z - prev_z) / (curr_z - prev_z);
				xzv_grid[xzv_curr] = prev_v - (prev_v * aab) + (curr_v * aab);
//				cout << "new val " << xzv_grid[xzv_curr] << endl;
			}
//			cout << "---" << endl;
		}
		cout << z_itr << " ";
	}
	cout << endl << "xtv to xtz conversion done, clean up" << endl;

	for(unsigned int i = 0; i < x * z; i++){
		vel_grid[i] = xzv_grid[i];
	}
//	long double* tp_ld;
//	tp_ld = vel_grid;
//	vel_grid = xzv_grid;
//	xzv_grid = tp_ld;

	longdouble2pgm("xzv_grid", xzv_grid, x, z);
	longdouble2pgm("vel_grid", vel_grid, x, z);

	delete [] xtz_grid;
	delete [] z_grid;
	delete [] xzv_grid;

	cout << "exiting velocity loading process" << endl << endl;
	return max_int_velocity;
}

bool import_xt(string filename, long double* &xt_grid, unsigned int x,
    unsigned int t){

	cout << "loading xt model" << endl;

	// place holder, use data from seg-y export
	//	for(unsigned int i = 0; i < (x*t); i++){
	//		xt_grid[i] = 100.0;
	//	}

	return load_array(filename, xt_grid, x, t);
}

int main(int argc, char *argv[]){

	cout << "Reverse Time Migration," << endl << "  build date/time "
	    << rtm_build_date << " " << rtm_build_time << endl
	    << "usage: rtm velocity_infile  xt_infile  dim_x  dim_t mul file_interval file_cutoff" << endl;

	// CMP 125~9824: 9700 inclusively
	unsigned int dim_x = atoi(argv[3]); // 9700;
	// 3000ms recording time is used for z scale
	unsigned int dim_t = atoi(argv[4]); // 3001;

	// z spacing for resulting grid
	long double dz;
	// z dimension for resultng grid
	unsigned int dim_z = 0; // impossible default

	// CMP spacing
	unsigned int d_cmp = 6.25;

	// sampling interval is 1ms
//	long double dt = 0.001;
	long double dt = atof(argv[6]); // 0.0005;
	cout << "dt:" << dt << endl;

	// total xt cell count
	unsigned int cell_count_t = dim_x * dim_t;

	// velocity model
//	long double* v_plane = new long double[cell_count_t];
	long double* v_plane = new long double[cell_count_t * 3]; //         hack for mem, estimating needed amount
	for(unsigned int i = 0; i < cell_count_t * 3; i++){
		v_plane[i]=0.0;
	}
	// return value used to determine file loading success
	long double return_value_v;
	// load up velocity model here
	string vel_if = argv[1];
	return_value_v = import_vel(vel_if, v_plane, dim_x, dim_t, atof(argv[5]), dt, dz, dim_z);
	cout << "maximum intermediate velocity from velocity analysis model = " << return_value_v
	    << endl;

	longdouble2pgm("v_plane", v_plane, dim_x, dim_z);


//	for(unsigned int i=0; i<dim_z; i++){
//		for(unsigned int j=0; j<dim_x; j++){
//			cout << v_plane[i*dim_x + j];
//		}
//		cout << endl;
//	}
//	cout << endl;

	// dx of grid, CMP spacing
	long double dx = d_cmp;

	// dz comes from sampling interval, dist = vel * time
	// later change this to use rms vel for vel
	// current 1500 as vel is to compare with fkmig
	// or just use dz = dx for square cells

	// max travel distance possible by waves
	long double max_travel = dz * dim_z;//(return_value_v * ((dim_t - 1) / 1000));
	cout << "maximum possible travel distance by wave in " << dim_t - 1
	    << " ms = " << max_travel << " meters." << endl
	    << " which should be the same as: " << return_value_v * atof(argv[5]) * (dim_z) / 1000 << endl;

	//	long double dz = 1500.0 * dt;
	//	long double dz = dx;

	// distribute total possible travel distance by time step counts
//	long double dz = max_travel / dim_t;

	/* att: this should not be processed here, but in loading velocity section */
	// integer ceiling to help risk of float exp overflow and computation cleanness
	//    dz = ceil(dz);
	// Multiplier to divert from exact velocity distance for dz
	// This can be to tap few away from 1.0 in order to avoid exp overflow of FP operations
	//		dz *= 1.1;

	cout << "max_travel = " << max_travel << endl;
	cout << "dim_t = " << dim_t << endl;
	cout << "dz = " << dz << endl;

	// redefine dim_z from velocity model
	// cells needed for total possible travel distance divided by dz
//	dim_z = ceil(max_travel / dz);
	cout << "dimension z = " << dim_z << " cells." << endl;

	// total xz cell count
	unsigned int cell_count_z = dim_x * dim_z;

	// source data: xt_plane
	long double* xt_plane = new long double[cell_count_t];
	// return value used to determine file loading success
	bool return_value_xt;
	// load up xt plane here
	string xt_if = argv[2];
	return_value_xt = import_xt(xt_if, xt_plane, dim_x, dim_t);

	// process cells in three time steps
	cout << "about to be start parsing: creating three time step grids" << endl;
	long double* xz_plane_0 = new long double[cell_count_z];
	long double* xz_plane_1 = new long double[cell_count_z];
	long double* xz_plane_2 = new long double[cell_count_z];

	// alias for three time steps cell planes
	// t_minus is target plane that is affected by t_zero and t_plus
	// these plane will rotate in time for reuse
	long double* t_minus = xz_plane_0;
	long double* t_zero = xz_plane_1;
	long double* t_plus = xz_plane_2;

	// init all processing planes to zero value, just in case
	for(unsigned int i = 0; i < cell_count_z; i++){
		t_minus[i] = 0.0;
		t_zero[i] = 0.0;
		t_plus[i] = 0.0;
	}

	cout << "pre-calc for dt, dx, dz, p_* etc" << endl;

	long double dtdx = dt / dx;
	long double dtdz = dt / dz;
	long double p_dtdx = pow(dtdx, 2);
	long double p_dtdz = pow(dtdz, 2);

	cout << "dtdx:" << dtdx << endl
			<< "dtdz:" << dtdz << endl
			<< "p_dtdx" << p_dtdx << endl
			<< "p_dtdz" << p_dtdz << endl
			<< endl;

	// pre-calculate square of velocity model
	cout << "pre-calc for square of velocity grid" << endl;
	for(unsigned int vl = 0; vl < dim_x * dim_z; vl++){
		v_plane[vl] = pow(v_plane[vl], 2);
	}
	cout << " ..is done." << endl;

	if(return_value_v && return_value_xt){

		cout << "now, starting kernel: " << endl;

		// target xtz index for center coordinate
//		unsigned int i_cc;

		// temp pointer for rotating three planes
		long double* t_tmp;

		// feed a row at t = rtime into z = 0 and run convolution
		// rtime starts at 3000 and ends at 0; total of 3001 cycles
		cout << "ReverseTimeStep:";
		for(int rtime = dim_t - 1; rtime >= 0; rtime--){
//			for(int rtime = dim_t - 1 - 1000; rtime >= 0; rtime--){

			cout << rtime << ".";

			// rotate three planes
			t_tmp = t_plus;
			t_plus = t_zero;
			t_zero = t_minus;
			t_minus = t_tmp;

			// feed a line from xt_plane
			memcpy(t_zero + dim_x * dim_t, xt_plane + (dim_x * rtime), dim_x * sizeof(long double));
//			memcpy(t_zero, xt_plane + (dim_x * (rtime + 2000)), dim_x * sizeof(long double));

			// convolution for all t_zero cells, except z = 0
			// iteration starts at 1 because z = 0 is not processed
			// boundary conditions are taken care of by different kernels

			// case mid band including LR edge (LR will be overwritten)
			for(unsigned int nxt = dim_x; nxt < dim_z * dim_x -dim_x; nxt++){
				t_minus[nxt] = 2 * t_zero[nxt] - t_plus[nxt] + v_plane[nxt] *
						(p_dtdx * (t_zero[nxt + 1] - 2 * t_zero[nxt] + t_zero[nxt - 1])
				        + p_dtdz * (t_zero[nxt - dim_x] - 2 * t_zero[nxt] + t_zero[nxt + dim_x]));
				if(std::isnan(t_minus[nxt])){
					cerr << "NaN: index= " << nxt << endl <<
							"\t\t\t" << "t_zero[nxt - dim_x] " << t_zero[nxt - dim_x] <<
							"\t" << "t_plus[nxt] " << t_plus[nxt] << endl <<
							"t_zero[nxt - 1] " << t_zero[nxt - 1] <<
							"\t" << "t_zero[nxt] " << t_zero[nxt] <<
							"\t\t" << "t_zero[nxt + 1] " << t_zero[nxt + 1] << endl <<
							"t_minus[nxt] " << t_minus[nxt] <<
							"\t" << "t_zero[nxt + dim_x] " << t_zero[nxt + dim_x] << endl <<
							endl <<
							"v_plane[nxt]" << v_plane[nxt] << endl <<
							t_zero[dim_x * (dim_t * 2 + dim_z)] << endl << endl;
					getchar();
				}
			}

//			for(unsigned int i = 0; i < cell_count_z; i++){
//				if(t_minus[i] < 0.0){
//					if(t_minus[i] <- pow(10.0, 7)){
//						t_minus[i] = -pow(10.0, 7);
//					}
//					if(t_minus[i] > -1){
//						t_minus[i] = -1;
//					}
//				}
//				else{
//					if(t_minus[i] > 0.0){
//						if(t_minus[i] > pow(10.0, 7)){
//							t_minus[i] = pow(10.0, 7);
//						}
//						if(t_minus[i] < 1){
//							t_minus[i] = 1;
//						}
//					}
//				}
//			}


			// write out pgm
			if(((rtime % atoi(argv[7])) == 0)&&(rtime <= atoi(argv[8]))){
				string filename = xt_if + "_RTMs_";
				std::string tmpStrng;
				std::stringstream tmpStrm;
				tmpStrm << setfill('0') << setw(5) << rtime;
				tmpStrng = tmpStrm.str();
				filename += tmpStrng;
//			if(!longdouble2pgm(filename, t_minus, dim_x, dim_z))
				if(!longdouble2pgm(filename, t_minus + dim_x * dim_t, dim_x, dim_z))
					cout << "error writing pgm:" << filename << endl;
			}

		} // end of iteration for reverse time

	} // condition of return_value_v && return_value_xt


	// clean up arrays.  Thank you arrays
	delete[] v_plane;
	delete[] xt_plane;

	delete[] xz_plane_0;
	delete[] xz_plane_1;
	delete[] xz_plane_2;


	cout << "exiting normally..." << endl;

	return 0;
}
