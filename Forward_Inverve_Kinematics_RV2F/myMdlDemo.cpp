#include <iostream>
#include <rl/math/Transform.h>
#include <rl/math/Unit.h>
#include <rl/mdl/Kinematic.h>
#include <rl/mdl/Model.h>
#include <rl/mdl/XmlFactory.h>
#include <rl/mdl/InverseKinematics.h>
#include <rl/mdl/NloptInverseKinematics.h>



int
main(int argc, char** argv)
{
	rl::mdl::XmlFactory factory;
	rl::mdl::Kinematic* kinematics = dynamic_cast<rl::mdl::Kinematic*>(factory.create("C:\\RoboWrapSVN4_build\\VC14_32\\dependencies\\rl-0.7.0\\share\\rl-0.7.0\\examples\\rlmdl\\mitsubishi-rv2f.xml"));
	rl::math::Vector q(6);
	q << 10, 10, -20, 30, 50, -10;
	q *= rl::math::DEG2RAD;
	kinematics->setPosition(q);
	kinematics->forwardPosition();
	rl::math::Transform t = kinematics->getOperationalPosition(0);
	rl::math::Vector3 position = t.translation();
	rl::math::Vector3 orientation = t.rotation().eulerAngles(2, 1, 0).reverse();
	std::cout << "Joint configuration in degrees: " << q.transpose() * rl::math::RAD2DEG << std::endl;
	std::cout << "End-effector position: [m] " << position.transpose() << " orientation [deg] " << orientation.transpose() * rl::math::RAD2DEG << std::endl;

	rl::mdl::NloptInverseKinematics ik(kinematics);
	ik.duration = std::chrono::seconds(1);
	ik.goals.push_back(::std::make_pair(t, 0)); // goal frame in world coordinates for first TCP
	bool result = ik.solve();
	rl::math::Vector solution = kinematics->getPosition();
	solution *= rl::math::RAD2DEG;
	for (int i = 0; i < solution.size(); i++)
	{
		std::cout << "Angles Iksolver: [m] " << solution[i] << std::endl;
	}
	

	int l;
	std::cin >> l;
	return 0;
}