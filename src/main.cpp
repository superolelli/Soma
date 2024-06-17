#include"Game\InitialState.hpp"


int main()
{
	CGameEngine engine;

	engine.Init("Soma");
	engine.PushStateImmediately(new CInitialState(&engine));
	engine.Run();

	engine.Quit();

	return 0;
}