__global__
void sumaKernel(float* a, float* b, float*c , int n){
    int index = blockIdx.x*blockDim.x+threadIdx.x;
    if (index<n)
    {
        c[index]=a[index]+b[index];
    }

}

void sumaVector(float* a, float* b, float*c , int n){
    int threads_per_block = 1024;
    int  num_blocks = std::ceil(n*1.0/threads_per_block);
    sumaKernel<<<num_blocks,threads_per_block>>>(a,b,c,n);

}
