/* Copyright (c) 2012 Jenner Hanni <jeh.wicker@gmail.com>
 * Licensed under the BSD three-clause license. See LICENSE.
 * 
 * Kalman filter algorithm came mostly from Dan Simon's article
 * in June 2001 issue of Embedded Systems Programming.
 */

#include "math.h"
#include "use_kf.h"

/* Use regular Kalman Filter */

void use_kf(float T, int duration) {

/* Step 1: Customize the Kalman Filter for your system
 * 'T' is time step and duration is total number of time steps.
 * Currently set up for 2 state, 1 input, 1 output system
 */

  int n = 2; // states
  int m = 1; // inputs
  int r = 1; // outputs

  int time;  // actual time displacement

  float position_noise = 10;  // feet
  float accel_noise = 0.2; // feet/sec^2

  float A[n][n]; // transition matrix 
  float B[n][m]; // input matrix
  float C[r][n]; // measurement matrix

  float A_T[n][n]; // transpose of A
  float C_T[2][1]; // transpose of C

  float xhat[n][m]; // state estimate containing position and velocity
  float x[n][m];    // state containing position and velocity

  float u = 1; // input: commanded acceleration in ft/sec^2
  float y;     // measured output

  float Sz = pow(position_noise,2); // measurement error covariance
  float Sw[n][n];                   // process noise covariance
  Sw[0][0] = pow(accel_noise,2) * pow(T,4)/4;
  Sw[0][1] = pow(accel_noise,2) * pow(T,3)/2;
  Sw[1][0] = pow(accel_noise,2) * pow(T,3)/2;
  Sw[1][1] = pow(accel_noise,2) * pow(T,2);

  float P_temp1[2][2];     // matrix: A * P * A_T
  float P_temp2[2][2];     // matrix: A * P * P_temp3 * P * A_T
  float P_temp3;           // scalar: C_T * 1/s * C

  float s;                 // covariance of the innovation vector
  float inn_var;           // innovation variable
  float w_k[n][m];         // process noise
  float z_k;               // measurement noise

  float K[2][1];           // Kalman Gain matrix

  float P[n][n];           // initial estimate covariance
  P[0][0] = Sw[0][0];  
  P[0][1] = Sw[0][1];
  P[1][0] = Sw[1][0];
  P[1][1] = Sw[1][1];

  float pos[duration];     // true position array
  float poshat[duration];  // estimated position array
  float posmeas[duration]; // measured position array
  float vel[duration];     // true velocity array
  float velhat[duration];  // estimated velocity array

  float state_temp[2][1];  // temporary array for matrix math
  float input_temp[2][1];  // temporary array for matrix math
  float gain_temp[2][2];
  int row = 0;
  int col = 0;
  int new_row = 0;
  int new_col = 0;

  printf("Hey, this works.\n");

  for (time = 0; time < duration; time++)
  {
    printf("time: %d, duration: %d\n",time,duration);

    /* Simulate the process noise */

    w_k[0][0] = accel_noise * pow(T,2)/2 * rand();
    w_k[0][1] = accel_noise * T * rand();

    state_temp[0][0] = A[0][0] * x[0][0] + A[0][1] * x[1][0];
    state_temp[1][0] = A[1][0] * x[0][0] + A[1][1] * x[1][0];

    input_temp[0][0] = B[0][0] * u;
    input_temp[1][0] = B[1][0] * u;

    x[0][0] = state_temp[0][0] + input_temp[0][0] + w_k[0][0];
    x[1][0] = state_temp[1][0] + input_temp[1][0] + w_k[1][0];

    /* Simulate the measurement noise */

    z_k = position_noise * rand();
    printf("z_k = %f\n",z_k);  // test, delete me

    y = (C[0][0] * x[0][0]) + (C[0][1] * x[1][0]) + z_k;

    /* Extrapolate most recent state estimate to the present time */
    /* xhat = A * xhat + B * u */

    state_temp[0][0] = A[0][0] * xhat[0][0] + A[0][1] * xhat[1][0];
    state_temp[1][0] = A[1][0] * xhat[0][0] + A[1][1] * xhat[1][0];

    input_temp[0][0] = B[0][0] * u;
    input_temp[1][0] = B[1][0] * u;

    xhat[0][0] = state_temp[0][0] + input_temp[0][0];

    /* Form innovation vector and compute its covariance */
    /* Inn = y - C * xhat; */
    /* s = C * P * transp(C) + Sz; */

    inn_var = y - C[0][0] * xhat[0][0] + C[0][1] * xhat[1][0];

    C_T[0][0] = 0;
    C_T[1][0] = 0;

    C[0][0] = 1;
    C[0][1] = 2;

    printf("C:   %f %f\n",C[0][0],C[0][1]);

    C_T[0][0] = C[0][0];
    C_T[1][0] = C[0][1];

    printf("C_T: %f\n     %f\n",C_T[0][0],C_T[1][0]);

    s = (C[0][0]*P[0][0] + C[1][0]*P[1][0]) * C_T[0][0];
    s = s + (C[0][0]*P[0][1] + C[1][0]*P[1][1]) * C_T[1][0];
    s = s + Sz;

    printf("s:   %f\n",s); 

    /* Calculate the Kalman Gain matrix */
    /* K = A * P * transp(C) * inv(s);  */

    gain_temp[0][0] = A[0][0]*P[0][0] + A[0][1]*P[1][0];
    gain_temp[0][1] = A[0][0]*P[0][1] + A[0][0]*P[0][1];
    gain_temp[1][0] = A[1][0]*P[0][0] + A[1][1]*P[1][0]; 
    gain_temp[1][1] = A[1][0]*P[0][1] + A[1][1]*P[1][1]; 

    K[0][0] = (1/s) * gain_temp[0][0]*C_T[0][0] + gain_temp[0][1]*C_T[1][0];
    K[1][0] = (1/s) * gain_temp[1][0]*C_T[0][0] + gain_temp[1][1]*C_T[1][0];

    printf("K:   %f\n     %f\n",K[0][0],K[1][0]);

    /* Update state estimate  */
    /* xhat = xhat + K * Inn  */

    xhat[0][0] = xhat[0][0] + K[0][0] * inn_var;
    xhat[1][0] = xhat[1][0] + K[1][0] * inn_var;

    /* Compute covariance of the estimation error                     */
    /* P = A*P*transp(A) - A*P*transp(C)*inv(s)*C*P*transp(A) + Sw;   */
    /* float P_temp1[2][2];     // matrix: A * P * A_T                */     
    /* float P_temp2[2][2];     // matrix: A * P * P_temp3 * P * A_T  */
    /* float P_temp3;           // scalar: C_T * 1/s * C              */
    
    A_T[0][0] = A[0][0]; // transpose A
    A_T[0][1] = A[1][0];
    A_T[1][0] = A[0][1];
    A_T[1][1] = A[1][1];

    P_temp1[0][0] = A[0][0]*P[0][0] + A[0][1]*P[1][0];
    P_temp1[0][1] = A[0][0]*P[0][1] + A[0][0]*P[0][1];
    P_temp1[1][0] = A[1][0]*P[0][0] + A[1][1]*P[1][0]; 
    P_temp1[1][1] = A[1][0]*P[0][1] + A[1][1]*P[1][1]; 

    P_temp1[0][0] = P_temp1[0][0]*A_T[0][0] + P_temp1[0][1]*A_T[1][0];
    P_temp1[0][1] = P_temp1[0][0]*A_T[0][1] + P_temp1[0][0]*A_T[0][1];
    P_temp1[1][0] = P_temp1[1][0]*A_T[0][0] + P_temp1[1][1]*A_T[1][0]; 
    P_temp1[1][1] = P_temp1[1][0]*A_T[0][1] + P_temp1[1][1]*A_T[1][1]; 

    P_temp3 = C_T[0][0] * C[0][0] + C_T[1][0] * C[0][1];
    P_temp3 = P_temp3 * (1/s);

    P_temp2[0][0] = (A[0][0]*P[0][0] + A[0][1]*P[1][0]) * P_temp3;
    P_temp2[0][1] = (A[0][0]*P[0][1] + A[0][0]*P[0][1]) * P_temp3;
    P_temp2[1][0] = (A[1][0]*P[0][0] + A[1][1]*P[1][0]) * P_temp3; 
    P_temp2[1][1] = (A[1][0]*P[0][1] + A[1][1]*P[1][1]) * P_temp3; 

    P_temp2[0][0] = P_temp2[0][0]*P[0][0] + P_temp2[0][1]*P[1][0];
    P_temp2[0][1] = P_temp2[0][0]*P[0][1] + P_temp2[0][0]*P[0][1];
    P_temp2[1][0] = P_temp2[1][0]*P[0][0] + P_temp2[1][1]*P[1][0]; 
    P_temp2[1][1] = P_temp2[1][0]*P[0][1] + P_temp2[1][1]*P[1][1]; 

    P_temp2[0][0] = P_temp2[0][0]*A_T[0][0] + P_temp2[0][1]*A_T[1][0];
    P_temp2[0][1] = P_temp2[0][0]*A_T[0][1] + P_temp2[0][0]*A_T[0][1];
    P_temp2[1][0] = P_temp2[1][0]*A_T[0][0] + P_temp2[1][1]*A_T[1][0]; 
    P_temp2[1][1] = P_temp2[1][0]*A_T[0][1] + P_temp2[1][1]*A_T[1][1]; 

    P[0][0] = P_temp1[0][0] - P_temp2[0][0] + Sw[0][0];
    P[0][1] = P_temp1[0][1] - P_temp2[0][1] + Sw[0][1];
    P[1][0] = P_temp1[1][0] - P_temp2[1][0] + Sw[1][0];
    P[1][1] = P_temp1[1][1] - P_temp2[1][1] + Sw[1][1];

    /* Add variables to the tracking arrays for later analysis */

    pos[time] = x[0][0];
    posmeas[time] = y;
    poshat[time] = xhat[0][0];
    vel[time] = x[1][0];
    velhat[time] = xhat[1][0];
  }

  printf("   pos         posmeas               poshat\n");
  for (time = 0; time < duration; time++)
  {
      printf("%d  %f  %f     %f\n",time,pos[time],posmeas[time],poshat[time]);
  }

}

