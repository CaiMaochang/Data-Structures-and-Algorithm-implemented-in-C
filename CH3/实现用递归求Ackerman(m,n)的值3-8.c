 #include<stdio.h>
 int ack(int m,int n)
 {
   int z;
   if(m==0) /* 出口 */
     z=n+1;
   else if(n==0)
     z=ack(m-1,1); /* 对形参m降阶 */
   else
     z=ack(m-1,ack(m,n-1)); /* 对形参m、n降阶 */
   return z;
 }

 int main()
 {
   int m,n;
   printf("Please input m,n:");
   scanf("%d,%d",&m,&n);
   printf("Ack(%d,%d)=%d\n",m,n,ack(m,n));
   
   return 0; 
 }
