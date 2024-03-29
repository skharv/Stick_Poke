#ifndef _OBJECTMANAGER_H
#define _OBJECTMANAGER_H

#include "GameObject.h"

class ObjectManager
{
private:
	std::vector<GameObject*> _objects;

	//when killing now, we want to instead add them to a list of dying entities.
	//then we will have them play their death animation and remove all collision.
	// at the end of their death animation, they will then be marked as Kill
	std::vector<GameObject*> _dyingObjects;

public:
	void Update(float Delta);
	void Draw(sf::RenderWindow &Window);

	void AddObject(GameObject* Object) { _objects.push_back(Object); };
	std::vector<GameObject*> GetObjects() { return _objects; };
	void PopulateDead(std::vector<GameObject*> &Vector);
	void KillDying(std::vector<GameObject*> &Vector);
	void KillAll(std::vector<GameObject*> &Vector);

	ObjectManager();
	~ObjectManager();
};

#endif