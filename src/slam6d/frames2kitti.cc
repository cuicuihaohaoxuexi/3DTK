/*
 * frames2riegl implementation
 *
 * Copyright (C) Shitong Du
 *
 * Released under the GPL version 3.
 *
 */
#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios;
#include "slam6d/globals.icc"
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include "XGetopt.h"
#endif

#if WIN32
#define snprintf sprintf_s
#endif 

int parseArgs(int argc,char **argv, char dir[255], int& start, int& end,int& sequence){
  start   = 0;
  end     = -1; // -1 indicates no limitation
  sequence=0;
  int  c;
  // from unistd.h
  extern char *optarg;
  extern int optind;

  cout << endl;
  while ((c = getopt (argc, argv, "s:e:q:")) != -1)
    switch (c)
   {
   case 's':
     start = atoi(optarg);
     if (start < 0) { cerr << "Error: Cannot start at a negative scan number.\n"; exit(1); }
     break;
   case 'e':
     end = atoi(optarg);
     if (end < 0)     { cerr << "Error: Cannot end at a negative scan number.\n"; exit(1); }
     if (end < start) { cerr << "Error: <end> cannot be smaller than <start>.\n"; exit(1); }
     break;

   case 'q':
     sequence = atoi(optarg);
     if (sequence < 0)     { cerr << "Error: Cannot end at a negative scan number.\n"; exit(1); }
     if (sequence > 21) { cerr << "Error: There are only 21 point clouds sequences.\n"; exit(1); }
     break;
   }

  if (optind != argc-1) {
    cerr << "\n*** Directory missing ***\n" << endl; 
    cout << endl
	  << "Usage: " << argv[0] << "  [-s NR] [-e NR] directory" << endl << endl;

    cout << "  -s NR   start at scan NR (i.e., neglects the first NR scans)" << endl
       << "          [ATTENTION: counting starts with 0]" << endl
	  << "  -e NR   end after scan NR" << "" << endl
	  << endl;
    cout << "Reads frame files from directory/scan???.frames and converts them to directory/scan???.4x4 in the RIEGL pose file format." << endl;
    abort();
  }
  strncpy(dir,argv[optind],255);

#ifndef _MSC_VER
  if (dir[strlen(dir)-1] != '/') strcat(dir,"/");
#else
  if (dir[strlen(dir)-1] != '\\') strcat(dir,"\\");
#endif
  return 0;
}


int main(int argc, char **argv)
{
  int start = 0, end =-1;
  int sequence=0;
  char dir[255];
  parseArgs(argc, argv, dir, start,end,sequence);
  int  fileCounter = start;
  char poseFileName[255];
  char frameFileName[255];
  ifstream pose_in;
  ofstream pose_out;
  snprintf(poseFileName,255,"%s%.2d.txt",dir,sequence);
  pose_out.open(poseFileName);
  pose_out.close();
  double inMatrix[12];
  double tMatrix[17];
  
  for (;;) {
    if (end > -1 && fileCounter > end) break; // 'nuf read
    snprintf(frameFileName,255,"%sscan%.3d.frames",dir,fileCounter++);
    snprintf(poseFileName,255,"%s%.2d.txt",dir,sequence);
   
    pose_in.open(frameFileName);

    if (!pose_in.good()) break; // no more files in the directory
    // read 3D scan

    cout << "Reading frame " << frameFileName << "..." << endl;
    
    while(pose_in.good()) {
      for (unsigned int i = 0; i < 17; pose_in >> tMatrix[i++]);
    }
    
  inMatrix[ 0] =tMatrix[0];
  inMatrix[ 1] =-tMatrix[4];
  inMatrix[ 2] =tMatrix[8];
  inMatrix[ 3] =tMatrix[12];
  inMatrix[ 4] =-(tMatrix[1]);
  inMatrix[ 5] =(tMatrix[5]);
  inMatrix[6] = -(tMatrix[9]);
  inMatrix[ 7] =-(tMatrix[13]);
  inMatrix[8] =tMatrix[2];
  inMatrix[ 9] =-tMatrix[6];
  inMatrix[10] =tMatrix[10];
  inMatrix[11] =tMatrix[14];
  inMatrix[12] =tMatrix[3];
  inMatrix[13] =tMatrix[7];;
  inMatrix[14] =tMatrix[11];
  inMatrix[15] =tMatrix[15];

    inMatrix[3] /= 100;
    inMatrix[7] /= 100;
    inMatrix[11] /= 100;
    
    pose_in.close();
    pose_in.clear();
    pose_out.open(poseFileName,ios::out | ios::app);

    cout << "Writing Kitti pose... " << poseFileName << endl;

    for (int i=0; i < 12; i++) {
      pose_out << inMatrix[i] << " ";
     // if((i % 4) == 3) pose_out << endl;
    }
    
    pose_out << endl;
    pose_out.close();
    pose_out.clear();

    
    
  }
    cout << " done." << endl;

    //pose_out.clear();
}
