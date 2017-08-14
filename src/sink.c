/* heap item count */
int N = 10;
int array[] = {1,2,3,4,5,6,7,8,9,0};

bool less(i, j)
{
  return array[i] < array[j];
}

void exechange(int i, int j)
{
  int temp;
  
  temp = array[i];
  array[i] = array[j];
  array[j] = temp;
  
  return;
}

void sink(int k)
{
  int j = k*2;
  
  while (j<N)
  {
    j = k*2;
    if (j<N && less(j, j+1))
    {
      j++;
    }
    /* array[k] >= array[j] means no need to recruit, can finish */
    if (!less(k, j))
    {
      break;
    }
    else
    {
      exechange(k, j);
      k = j;
    }
  }
}
