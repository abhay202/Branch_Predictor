#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
    
}bp_params;

int predictions = 0;
int mispredictions = 0;

unsigned char *bimodal_p_table = NULL;
unsigned char *gshare_p_table = NULL;
    
unsigned long int BHR = 0; 
unsigned long int BHR_mask = 0; 

unsigned char *chooser_table = NULL;

// Put additional data structures here as per your requirement

#endif
