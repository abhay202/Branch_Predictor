#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "sim_bp.h"

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // Look at sim_bp.h header file for the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    params.bp_name  = argv[1];

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

// initlize the tables for bimodal
    if(strcmp(params.bp_name, "bimodal") == 0){
        bimodal_p_table = (unsigned char *)malloc((1 << params.M2) * sizeof(unsigned char));
        for(int i = 0; i < (1 << params.M2); i++){
            bimodal_p_table[i] = 2;
        }
    }

// initlize the tables for gshare

    else if(strcmp(params.bp_name, "gshare") == 0){
        gshare_p_table = (unsigned char *)malloc((1 << params.M1)* sizeof(unsigned char));
        for(int i = 0; i < (1 << params.M1); i++) {
            gshare_p_table[i] = 2;
        }
    }

// initlize the tables for hybrid

    else if(strcmp(params.bp_name, "hybrid") == 0){

        bimodal_p_table = (unsigned char *)malloc((1 << params.M2)* sizeof(unsigned char));
        for(int i = 0; i < (1 << params.M2); i++){
            bimodal_p_table[i] = 2;
        }

        gshare_p_table = (unsigned char *)malloc((1 << params.M1) * sizeof(unsigned char));
        for(int i = 0; i < (1 << params.M1); i++){
            gshare_p_table[i] = 2;
        }

        chooser_table = (unsigned char*)malloc((1 << params.K) * sizeof(unsigned char));
        for(int i = 0; i < (1 << params.K) ; i++){
            chooser_table[i] = 1;
        }
    }

    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        outcome = str[0]; 
        predictions++;
//hybrid brnach prediction
        if(strcmp(params.bp_name, "hybrid") == 0){
            unsigned long int bimodal_branch_index = (addr >> 2) & ((1<<params.M2)- 1);
            unsigned char bimodal_counter = bimodal_p_table[bimodal_branch_index];

            char bimodal_prediction ;
            if (bimodal_counter >= 2) {
                bimodal_prediction = 't';
            } 
            else {
                bimodal_prediction = 'n';
            }
            
            int pc_index = (addr >> 2) & ((1 << params.M1) - 1);
            int upper_bits = (pc_index >> (params.M1 - params.N)) & ((1 << params.N) - 1);
            int lower_bits = pc_index & ((1 << (params.M1 - params.N)) - 1);
            int gshare_branch_index = ((upper_bits ^ BHR) << (params.M1 - params.N)) | lower_bits;
            char gshare_counter = gshare_p_table[gshare_branch_index];

            char gshare_prediction ;
            if (gshare_counter >= 2) {
                gshare_prediction = 't';
            } 
            else {
                gshare_prediction = 'n';
            }

            int chooser_index = (addr >> 2) & ((1 << params.K) - 1);
            char chooser_counter = chooser_table[chooser_index];

            char final_prediction;
            if(chooser_counter >= 2){
                final_prediction = gshare_prediction;
            }
            else{
                final_prediction = bimodal_prediction;
            }

            if(final_prediction != outcome){
                mispredictions++;
            }

            if(chooser_counter >= 2){
                if(outcome == 't'){
                    if(gshare_counter < 3)
                        gshare_p_table[gshare_branch_index]++;
                }
                else{
                    if(gshare_counter > 0)
                        gshare_p_table[gshare_branch_index]--;
                }
            }
            else{
                if(outcome == 't'){
                    if(bimodal_counter < 3)
                        bimodal_p_table[bimodal_branch_index]++;
                }
                else{
                    if(bimodal_counter > 0)
                        bimodal_p_table[bimodal_branch_index]--;
                }
            }

            if(params.N != 0){
                BHR_mask = (1 << params.N) - 1;
                unsigned long int one_shift_BHR = BHR>>1;
                int BHR_outcome;

                if(outcome == 't'){
                    BHR_outcome= 1;
                }else{
                    BHR_outcome = 0;
                }

                unsigned long int shifted_BHR = BHR_outcome<<(params.N-1);
                BHR = (one_shift_BHR | shifted_BHR ) & BHR_mask;

            }else{
                BHR_mask = 0;
            }

            int gshare_correct = (gshare_prediction == outcome);
            int bimodal_correct = (bimodal_prediction == outcome);

            if(gshare_correct != bimodal_correct){
                if(gshare_correct){
                    if(chooser_counter < 3)
                        chooser_table[chooser_index]++;
                }
                else{
                    if(chooser_counter > 0)
                        chooser_table[chooser_index]--;
                }
            }
        }
// gshare branch prediction for n>0 
        else if(strcmp(params.bp_name, "gshare") == 0){
            unsigned long int pc_index = (addr >> 2) & ((1 << params.M1) - 1);
            unsigned long int upper_bits = (pc_index >> (params.M1 - params.N)) & ((1 << params.N) - 1);
            unsigned long int lower_bits = pc_index & ((1 << (params.M1 - params.N)) - 1);
            unsigned long int branch_index = ((upper_bits ^ BHR) << (params.M1 - params.N)) | lower_bits;
            unsigned char counter = gshare_p_table[branch_index];

            char prediction ;
            if (counter >= 2) {
                prediction = 't';
            } 
            else {
                prediction = 'n';
            }

            if(prediction != outcome) {
                mispredictions++;
            }

            if(outcome == 't'){
                if(counter < 3)
                    gshare_p_table[branch_index]++;
            }
            else if(outcome == 'n'){
                if(counter > 0)
                    gshare_p_table[branch_index]--;
            }
            else{
                exit(EXIT_FAILURE);
            }
            
            if(params.N != 0){
                BHR_mask = (1 << params.N) - 1;
                unsigned long int one_shift_BHR = BHR>>1;
                int BHR_outcome;

                if(outcome == 't'){
                    BHR_outcome= 1;
                }else{
                    BHR_outcome = 0;
                }

                unsigned long int shifted_BHR = BHR_outcome<<(params.N-1);
                BHR = (one_shift_BHR | shifted_BHR ) & BHR_mask;

            }else{
                BHR_mask = 0;
            }
        }
// bimodal branch prediction for n=0 
        else if(strcmp(params.bp_name, "bimodal") == 0){
            unsigned long int branch_index = (addr >> 2) & ((1<<params.M2)- 1);
            unsigned char counter = bimodal_p_table[branch_index];
            char prediction ;
            if (counter >= 2) {
                prediction = 't';
            } 
            else {
                prediction = 'n';
            }

            if(prediction != outcome){
                mispredictions++;
            }

            if(outcome == 't'){
                if(counter < 3)
                    bimodal_p_table[branch_index]++;
            }
            else if(outcome == 'n'){
                if(counter > 0)
                    bimodal_p_table[branch_index]--;
            }
            else{
                exit(EXIT_FAILURE);
            }
        }

    }

    printf("OUTPUT\n");
    printf("number of predictions: %d\n", predictions);
    printf("number of mispredictions:%d \n", mispredictions);
    float misprediction_rate = ((float)mispredictions / predictions) * 100.0;
    printf("misprediction rate: %.2f%%\n", misprediction_rate);

    if(strcmp(params.bp_name, "hybrid") == 0){
        printf("FINAL CHOOSER CONTENTS\n");
        for(int i = 0; i < 1<<params.K; i++){
            printf("%d %d\n", i, chooser_table[i]);
        }
        printf("FINAL GSHARE CONTENTS\n");
        for(int i = 0; i < 1 << params.M1 ; i++){
            printf("%d %d\n", i, gshare_p_table[i]);
        }
        printf("FINAL BIMODAL CONTENTS\n");
        for(int i = 0; i < 1 << params.M2 ; i++)
        {
            printf("%d %d\n", i, bimodal_p_table[i]);
        }
    }

    else if(strcmp(params.bp_name, "gshare") == 0){
        printf("FINAL GSHARE CONTENTS\n");
        for(int i = 0; i < (1<<params.M1); i++){
            printf("%d %d\n", i, gshare_p_table[i]);
        }
    }

    else if(strcmp(params.bp_name, "bimodal") == 0){
        printf("FINAL BIMODAL CONTENTS\n");
        for( int i = 0; i <(1 << params.M2); i++)
        {
            printf("%d %d\n", i, bimodal_p_table[i]);
        }
    }
    
    fclose(FP);
    return 0;
}
