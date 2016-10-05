/*
 * CPASSREF/verify.c
 *
 *  Copyright 2013 John M. Schanck
 *
 *  This file is part of CPASSREF.
 *
 *  CPASSREF is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CPASSREF is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CPASSREF.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>

#include "constants.h"
#include "pass_types.h"
#include "poly.h"
#include "hash.h"
#include "formatc.h"
#include "ntt.h"
#include "pass.h"


#define CLEAR(f) memset((f), 0, PASS_N*sizeof(int64))

//int
//circ_conv(int64 *c, const int64 *a, const int64 *b)
//{
//	 int i,j,k;
//	 c[0]=0;
//	 int64 d[PASS_N];
//	 int64 x2[PASS_N];
//
//	 d[0]=b[0];
//
//	for(j=1;j<PASS_N;j++)            /*folding h(n) to h(-n)*/
//		d[j]=b[PASS_N -j];
//
//	 for(i=0;i<PASS_N;i++)
//		 c[0] += a[i]*d[i];
//
//	for(k=1;k<PASS_N;k++)
//	{
//				c[k]=0;
//				/*circular shift*/
//
//				for(j=1;j<PASS_N;j++)
//					x2[j]=d[j-1];
//				x2[0]=d[PASS_N-1];
//				for(i=0;i<PASS_N;i++)
//				{
//							d[i]=x2[i];
//							c[k]+=a[i]*x2[i];
//				}
//				c[k] = c[k]/PASS_b; //this should be q, check again (might be source of error)
//	}
//	return 0;
//}

int
verify(const unsigned char *h, const int64 *z, const int64 *pubkey,
    const unsigned char *message, const int msglen)
{
  int i,counter=0;
  b_sparse_poly c;
  int64 Fc[PASS_N] = {0};
  int64 Fz[PASS_N] = {0};
  unsigned char msg_digest[HASH_BYTES];
  unsigned char h2[HASH_BYTES];

  //changes
  int64 t[PASS_N]={0};

  //if(reject(z))
    //return INVALID;

  CLEAR(c.val);
  formatc(&c, h);

  ntt(Fc, c.val);
  ntt(Fz, z);


  for(i=0; i<PASS_t; i++) {
    Fz[S[i]] -= Fc[S[i]] * pubkey[S[i]];
  }

  poly_cmod(Fz);

  crypto_hash_sha512(msg_digest, message, msglen);
  hash(h2, Fz, msg_digest);

  for(i=0; i<HASH_BYTES; i++) {
    if(h2[i] != h[i])
    	counter++;
      //return INVALID;
  }

  /*
     * Classical NTRUSign verify
     */

    bsparseconv(t,pubkey,&c);
    //int i;
    // hash has to encoded to N bytes, make changes to add this technique (might be reason for failure of some sigs)
    // also check value of PASS_b -> q
    for(i=0;i<PASS_N;i++)
    {
  	  t[i] = (t[i] - h[i])/PASS_b; // (s*h-m)mod q
    }

    for(i=0;i<PASS_N;i++)
    {
    	if(t[i] > PASS_t)
    	{
    		counter++;
    		//printf("\nSignature Authentication FAilure!!\n");
    		//break;
    	}
    }
    //end of changes


  return VALID;
}

