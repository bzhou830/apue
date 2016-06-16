#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;




int main()
{
	int num = 0;
	vector<int> tm;
	vector<int> b;
	int tmp = 0;
	while(cin>>num)
	{
		
		for(int i=0; i<num; ++i)
		{
			cin >> tmp;	
			tm.push_back(tmp);					
		}

		sort(tm.begin(), tm.end());
		b.resize( *(tm.back()) - *(tm.begin()) + 2);
		tm.clear();
		b.clear();		

	}
	
	return 0;
}

