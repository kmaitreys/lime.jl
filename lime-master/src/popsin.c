/*
 *  popsin.c
 *  This file is part of LIME, the versatile line modeling engine
 *
 *  See ../COPYRIGHT
 *
***TODO:
	- Change the definition of the file format so that nmol is now written with the other mol[] scalars.
 */

#include "lime.h"
#include "defaults.h"


void
popsin(configInfo *par, struct grid **gp, molData **md, int *popsdone){
  FILE *fp;
  int i,j,k;
  double dummy;
  struct cell *dc=NULL; /* Not used at present. */
  unsigned long numCells,nExtraSinks;

  (void)dummy;

  if((fp=fopen(par->restart, "rb"))==NULL){
    if(!silent) bail_out("Error reading binary output populations file!");
exit(1);
  }

  par->numDensities = 1;
  checkFread(fread(&par->radius,   sizeof(double), 1, fp), 1, "par->radius");
  checkFread(fread(&par->ncell,    sizeof(int), 1, fp), 1, "par->ncell");
  if(par->ncell != (par->pIntensity + par->sinkPoints)){
    if(!silent) bail_out("Num grid points read from file != par->pIntensity + par->sinkPoints.");
exit(1);
  }

  checkFread(fread(&par->nSpecies, sizeof(int), 1, fp), 1, "par->nSpecies");
  if( par->nSpecies < 0 || par->nSpecies > MAX_NSPECIES )
    {
      if(!silent) bail_out("Error reading binary output populations file : is this really a binary output file generated by lime ?");
exit(1);
    }

  *md=realloc(*md, sizeof(molData)*par->nSpecies);

  for(i=0;i<par->nSpecies;i++){
    sprintf((*md)[i].molName, "unknown_%d", i+1);
    (*md)[i].amass = -1.0;

    (*md)[i].eterm = NULL;
    (*md)[i].gstat = NULL;
    (*md)[i].cmb = NULL;
    (*md)[i].gir = NULL;
    checkFread(fread(&(*md)[i].nlev,  sizeof(int),        1,fp), 1, "nlev");
    checkFread(fread(&(*md)[i].nline, sizeof(int),        1,fp), 1, "nline");
    checkFread(fread(&(*md)[i].npart, sizeof(int),        1,fp), 1, "npart");
    (*md)[i].part = malloc(sizeof(*((*md)[i].part))*(*md)[i].npart);
    for(j=0;j<(*md)[i].npart;j++){
      setCollPartsDefaults(&((*md)[i].part[j]));
      checkFread(fread(&(*md)[i].part[j].ntrans, sizeof(int), 1,fp), 1, "ntrans");
    }
    (*md)[i].lal=malloc(sizeof(int)*(*md)[i].nline);
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&(*md)[i].lal[j],    sizeof(int), 1,fp), 1, "lal");
    (*md)[i].lau=malloc(sizeof(int)*(*md)[i].nline);
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&(*md)[i].lau[j],    sizeof(int), 1,fp), 1, "lau");
    (*md)[i].aeinst=malloc(sizeof(double)*(*md)[i].nline);
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&(*md)[i].aeinst[j], sizeof(double), 1,fp), 1, "aeinst");
    (*md)[i].freq=malloc(sizeof(double)*(*md)[i].nline);
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&(*md)[i].freq[j],   sizeof(double), 1,fp), 1, "freq");
    (*md)[i].beinstl=malloc(sizeof(double)*(*md)[i].nline);
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&(*md)[i].beinstl[j],sizeof(double), 1,fp), 1, "beinstl");
    (*md)[i].beinstu=malloc(sizeof(double)*(*md)[i].nline);
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&(*md)[i].beinstu[j],sizeof(double), 1,fp), 1, "beinstu");
    for(j=0;j<(*md)[i].nline;j++) checkFread(fread(&dummy,sizeof(double), 1,fp), 1, "dummy");
    checkFread(fread(&dummy, sizeof(double),      1,fp), 1, "dummy");
    checkFread(fread(&dummy, sizeof(double),      1,fp), 1, "dummy");
  }

  mallocAndSetDefaultGrid(gp, (size_t)par->ncell, (size_t)par->nSpecies);

  for(i=0;i<par->ncell;i++){
    checkFread(fread(&(*gp)[i].id, sizeof (*gp)[i].id, 1, fp), 1, "id");
    checkFread(fread(&(*gp)[i].x, sizeof (*gp)[i].x, 1, fp), 1, "x");
    checkFread(fread(&(*gp)[i].vel, sizeof (*gp)[i].vel, 1, fp), 1, "vel");
    checkFread(fread(&(*gp)[i].sink, sizeof (*gp)[i].sink, 1, fp), 1, "sink");
    for(j=0;j<par->nSpecies;j++)
      checkFread(fread(&(*gp)[i].mol[j].nmol, sizeof(double), 1, fp), 1, "nmol");
    checkFread(fread(&(*gp)[i].dopb_turb, sizeof (*gp)[i].dopb_turb, 1, fp), 1, "dopb_turb");
    for(j=0;j<par->nSpecies;j++){
      (*gp)[i].mol[j].pops=malloc(sizeof(double)*(*md)[j].nlev);
      for(k=0;k<(*md)[j].nlev;k++) checkFread(fread(&(*gp)[i].mol[j].pops[k], sizeof(double), 1, fp), 1, "pops");
      for(k=0;k<(*md)[j].nline;k++) checkFread(fread(&dummy, sizeof(double), 1, fp), 1, "knu"); /* knu */
      for(k=0;k<(*md)[j].nline;k++) checkFread(fread(&dummy, sizeof(double), 1, fp), 1, "dust"); /* dust */
      checkFread(fread(&(*gp)[i].mol[j].dopb,sizeof(double), 1, fp), 1, "dopb");
      checkFread(fread(&(*gp)[i].mol[j].binv,sizeof(double), 1, fp), 1, "binv");
    }
    checkFread(fread(&dummy, sizeof(double), 1, fp), 1, "dummy");
    checkFread(fread(&dummy, sizeof(double), 1, fp), 1, "dummy");
    checkFread(fread(&dummy, sizeof(double), 1, fp), 1, "dummy");
  }
  fclose(fp);

/*
2017-06-21 IMS: Note that we have a bit of an issue with knu and dust here. These values are stored in the par->restart file for the frequencies of all the spectral lines, but what we actually need in raytrace are the values of knu and dust appropriate to the nominal continuum frequency of the image, which will not always be the same as that of any of the spectral lines. Probably the best thing would be to write some sort of interpolation routine, read in the line-frequency knu and dust values (which we are presently discarding), then call the interpolation routine within raytrace() as an alternative to calcGridContDustOpacity(). If this was done, the necessity to supply a dust file to par->dust, as well as density and temperature functions as below, would be avoided. However this is a bit more hacking than I presently want to contemplate.
*/

  delaunay(DIM, *gp, (unsigned long)par->ncell, 0, 1, &dc, &numCells);

  /* We just asked delaunay() to flag any grid points with IDs lower than par->pIntensity (which means their distances from model centre are less than the model radius) but which are nevertheless found to be sink points by virtue of the geometry of the mesh of Delaunay cells. Now we need to reshuffle the list of grid points, then reset par->pIntensity, such that all the non-sink points still have IDs lower than par->pIntensity.
  */ 
  nExtraSinks = reorderGrid((unsigned long)par->ncell, *gp);
  par->pIntensity -= nExtraSinks;
  par->sinkPoints += nExtraSinks;

  distCalc(par, *gp);
  if(!bitIsSet(defaultFuncFlags, USERFUNC_velocity)){
    /* In any case, it is extremely dodgy to read vertex velocities from a file but then use a separate-supplied function to calculate the edge velocity samples. This is stupid astronomer code - asking for trouble. */
    getEdgeVelocities(par,*gp);
    par->dataFlags |= (1 << DS_bit_ACOEFF);
  }

  par->dataFlags |= (1 << DS_bit_x);
  par->dataFlags |= (1 << DS_bit_neighbours);
  par->dataFlags |= (1 << DS_bit_velocity);
  par->dataFlags |= (1 << DS_bit_abundance);
  par->dataFlags |= (1 << DS_bit_turb_doppler);
/*  par->dataFlags |= (1 << DS_bit_magfield); commented out because we are not yet reading it in popsin (and may never do so) */
  par->dataFlags |= (1 << DS_bit_populations);

  if(bitIsSet(defaultFuncFlags, USERFUNC_density)){
    if(!silent) bail_out("You need to supply a density() function.");
exit(1);
  }
  if(bitIsSet(defaultFuncFlags, USERFUNC_temperature)){
    if(!silent) bail_out("You need to supply a temperature() function.");
exit(1);
  }

  for(i=0;i<par->ncell; i++)
    (*gp)[i].dens = malloc(sizeof(double)*par->numDensities);
  for(i=0;i<par->pIntensity;i++)
    density((*gp)[i].x[0],(*gp)[i].x[1],(*gp)[i].x[2],(*gp)[i].dens);
  for(i=par->pIntensity;i<par->ncell;i++){
    for(j=0;j<par->numDensities;j++)
      (*gp)[i].dens[j]=EPS; //************** what is the low but non zero value for? Probably to make sure no bad things happen in case something gets divided by this?
  }

  par->dataFlags |= DS_mask_density;

  for(i=0;i<par->pIntensity;i++)
    temperature((*gp)[i].x[0],(*gp)[i].x[1],(*gp)[i].x[2],(*gp)[i].t);
  for(i=par->pIntensity;i<par->ncell;i++){
    (*gp)[i].t[0]=par->tcmb;
    (*gp)[i].t[1]=par->tcmb;
  }

  par->dataFlags |= DS_mask_temperatures;

  *popsdone=1;
  par->useAbun = 0;

  free(dc);
}
