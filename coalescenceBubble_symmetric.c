/* Title: Coalescence of drops and bubbles
Bubble coalescence in a symmetric case. 

# Version 1.0
# Last update: Oct 8, 2024

# Author: Vatsal Sanjay
# vatsalsanjay@gmail.com
# Physics of Fluids
*/

// 1 is bubble
#include "axi.h"
#include "navier-stokes/centered.h"
#define FILTERED
#include "two-phase.h"
#include "navier-stokes/conserving.h"
#include "tension.h"
#include "distance.h"

int MAXlevel; // command line input

#define tsnap (1e-3)
#define tsnap2 (1e-4)
// Error tolerances
#define fErr (1e-3)                                 // error tolerance in VOF
#define VelErr (1e-2)                            // error tolerances in velocity

// boundary conditions

// Other command - line inputs.
double tmax, OhIn, OhOut, RhoIn;
double Ldomain; // Dimension of the domain
char nameOut[80], dumpFile[80];

int main(int argc, char const *argv[]) {
  if (argc < 6){
    fprintf(ferr, "Lack of command line arguments. Check! Need %d more arguments\n", 6-argc);
    return 1;
  }

  // Values taken from the terminal

  OhOut = atof(argv[1]);
  OhIn = atof(argv[2]);
  RhoIn = atof(argv[3]);
  MAXlevel = atoi(argv[4]);
  tmax = atof(argv[5]);

  Ldomain = 6.0;

  fprintf(ferr, "Level %d, Ldomain %g, tmax %3.2f, OhIn %3.2e, OhOut %3.2e, Rho21 %4.3f, SYM\n", MAXlevel, Ldomain, tmax, OhIn, OhOut, RhoIn);

  L0=Ldomain;
  origin(0.0, 0.0);
  init_grid (1 << (6));

  rho1 = RhoIn; mu1 = OhIn;
  rho2 = 1e0; mu2 = OhOut;
  f.sigma = 1.0;

  char comm[80];
  sprintf (comm, "mkdir -p intermediate");
  system(comm);
  sprintf (dumpFile, "dump");

  run();
}

event init(t = 0){
  if (!restore (file = dumpFile)){
    char filename[60];
    sprintf(filename,"InitialCondition.dat");

    FILE * fp = fopen(filename,"rb");
    if (fp == NULL){
      fprintf(ferr, "There is no file named %s\n", filename);
      return 1;
    }
    coord* InitialShape;
    InitialShape = input_xy(fp);
    fclose (fp);
    scalar d[];
    distance (d, InitialShape);
    while (adapt_wavelet ((scalar *){f, d}, (double[]){1e-8, 1e-8}, MAXlevel).nf);
    /**
    The distance function is defined at the center of each cell, we have
    to calculate the value of this function at each vertex. */
    vertex scalar phi[];
    foreach_vertex(){
      phi[] = (d[] + d[-1] + d[0,-1] + d[-1,-1])/4.;
    }
    /**
    We can now initialize the volume fraction of the domain. */
    fractions (phi, f);
    foreach(){
    u.x[] = 0.0;
    u.y[] = 0.0;
    // p[] = 2*(1.-f[]);
    }
    dump (file = dumpFile);
    static FILE * fp2;
    fp2 = fopen("InitialConditionStatus.dat","w");
    fprintf(fp2, "Initial condition is written to %s\n", dumpFile);
    fclose(fp2);
    // return 1;
  }
}

event adapt(i++){
  adapt_wavelet ((scalar *){f, u.x, u.y},
     (double[]){fErr, VelErr, VelErr},
      MAXlevel, MAXlevel-6);
}

// Outputs
event writingFiles (t = 0; t += tsnap; t <= tmax+tsnap) {
  dump (file = dumpFile);
  sprintf (nameOut, "intermediate/snapshot-%5.4f", t);
  dump(file=nameOut);
}

event end (t = end) {
  fprintf(ferr, "Level %d, Ldomain %g, tmax %3.2f, OhIn %3.2e, OhOut %3.2e, Rho21 %4.3f, SYM\n", MAXlevel, Ldomain, tmax, OhIn, OhOut, RhoIn);
}

event logWriting (t = 0; t += tsnap2; t <= tmax+tsnap) {

  double ke = 0.;

  foreach (reduction(+:ke)){
    ke += 2*pi*y*(0.5*clamp(f[], 0., 1.)*(sq(u.x[]) + sq(u.y[])))*sq(Delta);
  }
  
  static FILE * fp;

  if (pid() == 0) {
    if (i == 0) {
      fprintf (ferr, "i dt t ke\n");
      fp = fopen ("log", "w");
      fprintf(fp, "Level %d, Ldomain %g, tmax %3.2f, OhIn %3.2e, OhOut %3.2e, Rho21 %4.3f, SYM\n", MAXlevel, Ldomain, tmax, OhIn, OhOut, RhoIn);
      fprintf (fp, "i dt t ke\n");
      fprintf (fp, "%d %g %g %g\n", i, dt, t, ke);
      fclose(fp);
    } else {
      fp = fopen ("log", "a");
      fprintf (fp, "%d %g %g %g\n", i, dt, t, ke);
      fclose(fp);
    }
    fprintf (ferr, "%d %g %g %g\n", i, dt, t, ke);
  }

  assert(ke > -1e-10);
  // dump (file = "dump");
  if (ke < 1e-5 && i > 1000){
    if (pid() == 0){
      fprintf(ferr, "kinetic energy too small now! Stopping!\n");
      fp = fopen ("log", "a");
      fprintf(fp, "kinetic energy too small now! Stopping!\n");
      fclose(fp);
    }
    return 1;
  }
}