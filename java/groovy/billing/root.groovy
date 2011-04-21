package billing;

import static billing.DataFields.*;

public class RootRule extends Rule {

    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        Rule rule = factory.create(type.getRuleName());
        return rule.getMoneyInPips(input);
    }
}
