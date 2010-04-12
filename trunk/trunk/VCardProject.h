#ifndef VCARDPROJECT_H
#define VCARDPROJECT_H

class VCard;
#include <QSet>

class VCardProject
{
public:
    enum Version
    {
        VER_2_1,
        VER_3_0
    };

    VCardProject();

    int getVCardCount() const;
    const VCard& getVCard(int index) const;

    void addVCard(const VCard& vCard);
    void updateVCard(int index, const VCard& vCard);
    void removeVCard(int index);

private:
    QSet<VCard*> m_vCardSet;
};

#endif // VCARDPROJECT_H
