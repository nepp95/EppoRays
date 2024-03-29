#include "AppLayer.h"

#include <EppoCore.h>
#include <EppoCore/Core/Entrypoint.h>

using namespace Eppo;

class EppoRays : public Application
{
public:
	EppoRays(const ApplicationSpecification &specification)
		: Application(specification)
	{
		std::shared_ptr<AppLayer> layer = std::make_shared<AppLayer>();

		PushLayer(layer);
	}

	~EppoRays() = default;
};

Application *Eppo::CreateApplication()
{
	ApplicationSpecification spec;
	spec.Title = "EppoRays";
	spec.Width = 1600;
	spec.Height = 900;

	return new EppoRays(spec);
}
