#include"Game\InitialState.hpp"


int main()
{
	//start the engine
	CGameEngine engine;

	engine.Init("Soma");
	engine.PushStateImmediately(new CInitialState());
	engine.Run();

	engine.Quit();

	return 0;
}