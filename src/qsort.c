void quicksort(int *a, int left,int right) 
{ 
	int i,j,t,temp; 

	if(left>right) 
		return; 

	temp=a[left]; //temp中存的就是基准数 
	i=left; 
	j=right; 
	while(i!=j) 
	{ 
		//顺序很重要，要先从右边开始找 
		while(a[j]>=temp && i<j) 
			j--;

		//再找左边的 
		while(a[i]<=temp && i<j) 
			i++;

		//交换两个数在数组中的位置 
		if(i<j) 
		{ 
			t=a[i]; 
			a[i]=a[j]; 
			a[j]=t; 
		} 
	}
	//最终将基准数归位 
	a[left]=a[i]; 
	a[i]=temp; 

	quicksort(a, left,i-1);//继续处理左边的，这里是一个递归的过程 
	quicksort(a,i+1,right);//继续处理右边的 ，这里是一个递归的过程 
} 


int main()
{
	int i;

	int array[] = {6,1,2,7,9,3,4,5,10,8};

	sort(array, 0, sizeof(array)/sizeof(array[0]) - 1);

	for (i=0;i<sizeof(array)/sizeof(array[0]); i++)
	{
		printf("%d ", array[i]);
	}
	printf("\n");
	getchar();

	return 0;
}
