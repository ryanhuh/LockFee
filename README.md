# LockFee
lock free using atomic_compare_exchange_strong

멀티 쓰레드 프로그램에서 Queue를 사용하는 경우 일반적으로 Lock 처리를 통해 동기화합니다. 
푸시하거나 팝하기 전에 Lock를 걸고 기능이 끝나기 직전에 Lock를 해제하는 것이죠. 
그러나 Lock은 약간의 오버헤드가 발생하므로, Lock을 사용하지 않으면서 동시 작업을 지원할 수 있다면 정말 좋을 것입니다. 
연결된 리스트, 스택, 큐와 같은 많은 데이터 구조의 경우, 원자 함수 비교 및 스왑(CAS)을 사용하여 동기화되고 Lock을 걸지 않는 버전을 만들 수 있습니다. 
C++에서는 atomic_compare_exchange_strong 로 구현이 가능하죠.

Reference
http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.53.8674&rep=rep1&type=pdf

