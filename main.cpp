#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
    std::vector<int> nums = {8, 5, 11, -3, 9, 22, 89, -11, 3, 2};

    int indexMax = nums.size() - 1;

    int ResIndexMax = nums.size() - 1;
    int ResIndexMin = nums.size() - 1;

    int maxDiff = 0;

    for(int i = nums.size() - 1; i >= 0; i--)
    {
        if(nums[i] > nums[indexMax]) indexMax = i;

        int tmpDiff = nums[indexMax] - nums[i];
        if(tmpDiff > maxDiff) {
            ResIndexMax = indexMax;
            ResIndexMin = i;
            maxDiff = tmpDiff;
        }
    }

    std::cout << ResIndexMax << " " << ResIndexMin << std::endl;

    return 0;
}