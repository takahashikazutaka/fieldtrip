/*
 * Copyright (C) 2010, Stefan Klanke
 * Donders Institute for Donders Institute for Brain, Cognition and Behaviour,
 * Centre for Cognitive Neuroimaging, Radboud University Nijmegen,
 * Kapittelweg 29, 6525 EN Nijmegen, The Netherlands
 */

#include <mex.h>
#include <matrix.h>
#include <siemensap.h>

mxArray *createStructFromSAP(sap_item_t *item) {
	mxArray *A;
	
	if (item==NULL) {
		return mxCreateStructMatrix(0,0,0,NULL);
	}
	
	A = mxCreateStructMatrix(1,1,0,NULL);
	
	while (item!=NULL) {
		mxArray *F = NULL;
		int nr;
		
		nr = mxAddField(A, item->fieldname);
		if (nr==-1) mexErrMsgTxt("Out of memory");
		
		switch(item->type) {
			case SAP_DOUBLE:
				F = mxCreateDoubleMatrix(item->num_elements, 1, mxREAL);
				memcpy(mxGetPr(F), item->value, item->num_elements*sizeof(double));
				break;
			case SAP_LONG:
				if (sizeof(long) == 4) {
					/* 32 bit machine */
					F = mxCreateNumericMatrix(item->num_elements, 1, mxINT32_CLASS, mxREAL);
				} else {
					F = mxCreateNumericMatrix(item->num_elements, 1, mxINT64_CLASS, mxREAL);
				}
				memcpy(mxGetPr(F), item->value, item->num_elements*sizeof(long));
				break;
			case SAP_TEXT:
				F = mxCreateString((char *) item->value);
				break;
			case SAP_STRUCT:
				if (item->is_array) {
					sap_item_t **children = (sap_item_t **) item->value;
					int i;
					/*  We need to use a cell array here, since we're not guaranteed to have 
						the same fields in each element
					*/
					F = mxCreateCellMatrix(item->num_elements,1);
					for (i=0;i<item->num_elements;i++) {
						mxArray *Fi = createStructFromSAP(children[i]);
						mxSetCell(F,i,Fi);
					}
				} else {
					sap_item_t **children = (sap_item_t **) item->value;
					F = createStructFromSAP(children[0]);
				}
				break;
		}
		if (F!=NULL) mxSetFieldByNumber(A,0,nr,F);
		
		item = item->next;
	}
	
	return A;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	char *buffer;
	int size;
	sap_item_t *L = NULL;
	
	if (nrhs!=1 || !mxIsChar(prhs[0])) mexErrMsgTxt("This function needs exactly one (string) argument.");
	
	size = mxGetNumberOfElements(prhs[0]);
	
	if (sizeof(char) == sizeof(mxChar))  {
		buffer = (char *) mxGetData(prhs[0]);
		L = sap_parse(buffer, size);
	} else {
		buffer = mxArrayToString(prhs[0]);
		L = sap_parse(buffer, size);
		mxFree(buffer);
	}
	
	plhs[0] = createStructFromSAP(L);
	
	sap_destroy(L);
}