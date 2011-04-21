package billing;

import static billing.DataFields.*;

public class NormalUserShortMessageRule extends Rule {

    public static final long kPipsPerShortMessage = 1000L;

    public static final long kPipsShortMessagePackage = 10*10000L;
    public static final long kNoChargeMessagesInPackage = 200L;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kNormal;

        List<Package> packages = input.getPackages();

        if (packages.contains(PackageType.kNormalUserShortMessage)) {
            boolean isNew = input.getBoolean(UserField.kIsNewUser);
            assert !isNew;
            Rule message = factory.create("package_short_message",
                 [ kPipsShortMessagePackage, kNoChargeMessagesInPackage, kPipsPerShortMessage ]);
            return message.getMoneyInPips(input);
        } else {
            long noChargeMessages = (Long)state;
            Rule message = factory.create("package_short_message",
                [ 0L, noChargeMessages, kPipsPerShortMessage ]);
            return message.getMoneyInPips(input);
        }
    }
}
