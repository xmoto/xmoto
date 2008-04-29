/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <stdlib.h>
#include <string.h>

#include "chipmunk.h"


#define CP_ARRAY_INCREMENT 10

// NOTE: cpArray is rarely used and will probably go away.

cpArray*
cpArrayAlloc(void)
{
	return (cpArray *)calloc(1, sizeof(cpArray));
}

cpArray*
cpArrayInit(cpArray *arr, int size)
{
	arr->num = 0;
	
	size = (size ? size : CP_ARRAY_INCREMENT);
	arr->max = size;
	arr->arr = (void **)malloc(size*sizeof(void**));
	
	return arr;
}

cpArray*
cpArrayNew(int size)
{
	return cpArrayInit(cpArrayAlloc(), size);
}

void
cpArrayDestroy(cpArray *arr)
{
	free(arr->arr);
}

void
cpArrayFree(cpArray *arr)
{
	if(!arr) return;
	cpArrayDestroy(arr);
	free(arr);
}

void
cpArrayPush(cpArray *arr, void *object)
{
	if(arr->num == arr->max){
		arr->max += CP_ARRAY_INCREMENT;
		arr->arr = (void **)realloc(arr->arr, arr->max*sizeof(void**));
	}
	
	arr->arr[arr->num] = object;
	arr->num++;
}

void
cpArrayDeleteIndex(cpArray *arr, int index)
{
	int last = --arr->num;
	arr->arr[index] = arr->arr[last];
}

void
cpArrayDeleteObj(cpArray *arr, void *obj)
{
	int i;
	for(i=0; i<arr->num; i++){
		if(arr->arr[i] == obj){
			cpArrayDeleteIndex(arr, i);
			return;
		}
	}
}

void
cpArrayEach(cpArray *arr, cpArrayIter iterFunc, void *data)
{
	int i;
	for(i=0; i<arr->num; i++)
		iterFunc(arr->arr[i], data);
}

int
cpArrayContains(cpArray *arr, void *ptr)
{
	int i;
	for(i=0; i<arr->num; i++)
		if(arr->arr[i] == ptr) return 1;
	
	return 0;
}
