/* 
 * File:   filter.c
 * Author: MATHRAX
 *
 * Created on March 23, 2018, 11:36 AM
 */

#include <xc.h>
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 

unsigned long sorted[BUFFER_LENGTH];
/*
 * 
 */
void bubbleSort(unsigned long A[], unsigned int len) {
    unsigned long newn;
    unsigned long n = len;
    unsigned long temp = 0;
    unsigned int p;
    do {
        newn = 1;
        for (p = 1; p < len; p++) {
            if (A[p - 1] > A[p]) {
                temp = A[p]; //swap places in array
                A[p] = A[p - 1];
                A[p - 1] = temp;
                newn = p;
            } //end if
        } //end for
        n = newn;
    } while (n > 1);
}



unsigned long median(unsigned long samples[], unsigned int m) //calculate the median
{
    
    for (unsigned int i = 0; i < m; i++) {
        sorted[i] = samples[i];
    }
    bubbleSort(sorted, m); // Sort the values

    if ((m & 0b00000001) == 1) { //If the last bit of a number is 1, it's odd. This is equivalent to "TRUE". Also use if m%2!=0.
        return sorted[m / 2]; //If the number of data points is odd, return middle number.
    } else {
        return (sorted[(m / 2) - 1] + sorted[m / 2]) / 2; //If the number of data points is even, return avg of the middle two numbers.
    }
}

