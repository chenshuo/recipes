package billing;

import static billing.DataFields.*;

public class NormalUserInternetRule extends Rule {

    public static final long kPipsPerKiloBytes = 50;

    public static final long kPipsPerKiloBytesWithPackage = 30;
    public static final long kPipsInternetPackage = 20*10000;
    public static final long kNoChargeKiloBytesInPackage = 50*1000;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kNormal;
        List<Package> packages = input.getPackages();

        if (packages.contains(PackageType.kNormalUserInternet)) {
            boolean isNew = input.getBoolean(UserField.kIsNewUser);
            assert !isNew;
            Rule message = factory.create("package_internet",
                [ kPipsInternetPackage, kNoChargeKiloBytesInPackage, kPipsPerKiloBytesWithPackage ]);
            return message.getMoneyInPips(input);
        } else {
            long noChargeKiloBytes = (Long)state;
            Rule message = factory.create("package_internet",
                [ 0L, noChargeKiloBytes, kPipsPerKiloBytes ]);
            return message.getMoneyInPips(input);
        }
    }
}
