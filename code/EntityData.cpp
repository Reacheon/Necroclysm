import EntityData;

EntityData::EntityData() = default;
EntityData::~EntityData() = default;
EntityData::EntityData(EntityData&&) = default;

EntityData EntityData::cloneEntity() const
{
    EntityData newEntity;

    static_cast<EntityData_Base&>(newEntity) = static_cast<const EntityData_Base&>(*this);

    return newEntity;
}
