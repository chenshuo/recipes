package billing;

import static billing.DataFields.*;

public class NormalUserRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kVip;
        boolean isNew = input.getBoolean(UserField.kIsNewUser);
        if (isNew) {
            Rule newUser = factory.create(type.getRuleName()+"_newjoiner");
            return newUser.getMoneyInPips(input);
        } else {
            List<Package> packages = input.getPackages();

            if (packages.contains(PackageType.kVipUserPackage1)) {
                Rule noPackage = factory.create(type.getRuleName()+"_package1");
                return noPackage.getMoneyInPips(input);
            } else if (packages.contains(PackageType.kVipUserPackage2)) {
                Rule noPackage = factory.create(type.getRuleName()+"_package2");
                return noPackage.getMoneyInPips(input);
            } else {
                Rule noPackage = factory.create(type.getRuleName()+"_nopackage", [ 0L, 0L, 0L ]);
                return noPackage.getMoneyInPips(input);
            }
        }
    }
}
