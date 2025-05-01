import EntityData;

EntityData::EntityData() = default;
EntityData::~EntityData() = default;
EntityData::EntityData(EntityData&&) = default;

EntityData EntityData::cloneForTransfer() const
{
    EntityData newEntity;

    static_cast<EntityData_Base&>(newEntity) = static_cast<const EntityData_Base&>(*this);

    return newEntity;
}
