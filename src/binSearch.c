

int binSearch(int *array, int start, int end, int key)
{
  int mid;
  
  while (start <= end)
  {
    mid = (start + end)/2;
    
    if (array[mid] > key)
    {
      start = mid + 1;
    }
    
    else if (array[mid] < key)
    {
      end = mid - 1;
    }
    else
    {
      return mid;
    }
  }
  
  return -1;
}
