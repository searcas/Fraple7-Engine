#pragma once

namespace Fraple7
{
	namespace Studio
	{
		class Keyboard
		{
		public:
			Keyboard();
			~Keyboard();
			void ReadKey(unsigned char key);
			bool EventHandler();
		private:
		};
	}
}
