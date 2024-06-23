#include"Game\InitialState.hpp"


int main()
{
	CGameEngine engine("Soma");
	engine.PushStateImmediately(new CInitialState(&engine));
	engine.Run();

	return 0;
}