#include <app.h>
#include <format_.h>

class True : public sys::app
{
public:

    int Run() override
    {
        return 0;
    }
};

sys::app* CreateApp()
{
    return new True();
}
