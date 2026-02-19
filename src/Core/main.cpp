#include "Application.h"
#include "ResourceManager.h"

int main() {
    ApplicationSpecification spec;
    spec.Name = "Calcium3D";
    spec.Width = 1200;
    spec.Height = 800;
    
    Application app(spec);
    app.Run();
    return 0;
}