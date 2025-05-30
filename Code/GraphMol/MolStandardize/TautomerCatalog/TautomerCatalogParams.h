//
//  Copyright (C) 2018-2021 Susan H. Leung and other RDKit contributors
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <RDGeneral/export.h>
#ifndef RD_TAUTOMER_CATALOG_PARAMS_H
#define RD_TAUTOMER_CATALOG_PARAMS_H

#include <Catalogs/CatalogParams.h>
#include <GraphMol/RDKitBase.h>
#include <string>
#include <vector>
#include <iostream>

namespace RDKit {
class ROMol;

namespace MolStandardize {

using TautomerTransformDefs =
    std::vector<std::tuple<std::string, std::string, std::string, std::string>>;

namespace defaults {
RDKIT_MOLSTANDARDIZE_EXPORT extern const TautomerTransformDefs
    defaultTautomerTransforms;
RDKIT_MOLSTANDARDIZE_EXPORT extern const TautomerTransformDefs
    defaultTautomerTransformsv1;
}  // namespace defaults

class RDKIT_MOLSTANDARDIZE_EXPORT TautomerTransform {
 public:
  ROMol *Mol = nullptr;
  std::vector<Bond::BondType> BondTypes;
  std::vector<int> Charges;

  TautomerTransform(ROMol *mol, std::vector<Bond::BondType> bondtypes,
                    std::vector<int> charges)
      : Mol(mol),
        BondTypes(std::move(bondtypes)),
        Charges(std::move(charges)) {}

  TautomerTransform(const TautomerTransform &other)
      : BondTypes(other.BondTypes), Charges(other.Charges) {
    Mol = new ROMol(*other.Mol);
  }

  TautomerTransform &operator=(const TautomerTransform &other) {
    if (this != &other) {
      delete Mol;
      Mol = new ROMol(*other.Mol);
      BondTypes = other.BondTypes;
      Charges = other.Charges;
    }
    return *this;
  }

  ~TautomerTransform() { delete Mol; }
};

class RDKIT_MOLSTANDARDIZE_EXPORT TautomerCatalogParams
    : public RDCatalog::CatalogParams {
 public:
  TautomerCatalogParams() {
    d_typeStr = "Tautomer Catalog Parameters";
    d_transforms.clear();
  }

  TautomerCatalogParams(const std::string &tautomerFile);
  TautomerCatalogParams(const TautomerTransformDefs &data);
  // copy constructor
  TautomerCatalogParams(const TautomerCatalogParams &other);

  ~TautomerCatalogParams() override;

  const std::vector<TautomerTransform> &getTransforms() const;

  const TautomerTransform getTransform(unsigned int fid) const;

  void toStream(std::ostream &) const override;
  std::string Serialize() const override;
  void initFromStream(std::istream &ss) override;
  void initFromString(const std::string &text) override;

 private:
  //		std::vector<std::pair<ROMol*, ROMol*>> d_pairs;
  std::vector<TautomerTransform> d_transforms;

};  // class TautomerCatalogParams

}  // namespace MolStandardize
}  // namespace RDKit

#endif
