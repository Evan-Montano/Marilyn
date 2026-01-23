#include <iostream>
#include "Marilyn.h"
#include "DataHelper.h"
#include "Train.h"
#include "Chat.h"

int main()
{
    int mode = 0;

    std::cout << "Select mode:\n";
    std::cout << "1) Chat\n";
    std::cout << "2) Train\n";
    std::cout << "> ";
    std::cin >> mode;

    switch (mode)
    {
        case 1:
            initChatModule();
            break;

        case 2:
            initTrainModule();
            break;

        default:
            std::cout << "Invalid selection.\n";
            return 1;
    }

    return 0;
}
