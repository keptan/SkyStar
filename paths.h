using namespace std;

// the length variable is how many total ms it should take to complete the
//   nodes, i couldn't think of a better name for it.

pos linearPath ( int ms_passed, int length, vector<pos> nodes )
{
	if ( ms_passed >= length ) return nodes.back();
	float segment_length = (float)length / ( nodes.size()-1 );
	int segment = 0;
	while ( (1 + segment) * segment_length < ms_passed ) segment++;
	ms_passed -= segment * segment_length;
	float t = ms_passed / segment_length;
	pos out;
	out.x = ( 1 - t ) * nodes[segment].x + t * nodes[segment+1].x;
	out.y = ( 1 - t ) * nodes[segment].y + t * nodes[segment+1].y;
	return out;
}

int binomial(int n, int k) {
	if(k<0 or k>n)return 0;
	if(k==0 or k==n)return 1;
	if(k>n-k)k=n-k;
	int c=1;
	for(int i=0;i<k;i++)c=c*(n-i)/(i+1);
	return c;
}

pos bezierPath( int ms_passed, int length, vector<pos> nodes )
{
	if ( ms_passed >= length ) return nodes.back();
	pos out;
	float t = 1.0 / length * ms_passed;
	int n = nodes.size()-1;
	for ( int i=0; i<=n; i++ ) {
		out.x += binomial( n,i ) * pow( 1-t, n-i ) * pow( t,i ) * nodes[i].x;
		out.y += binomial( n,i ) * pow( 1-t, n-i ) * pow( t,i ) * nodes[i].y;
	}
	return out;
}

vector<pos> generateRandomNodes( int num_of_nodes = 3,
			       int min_x = 0, int max_x = 80,
			       int min_y = 0, int max_y = 80 ) 
{
	vector<pos> nodes;
	pos p;
	for (int i = 0; i < num_of_nodes; i++)
	{
		p.x = std::experimental::randint(min_x,max_x);
		p.y = std::experimental::randint(min_y,max_y);
		nodes.push_back(p);
	}
	return nodes;
}
