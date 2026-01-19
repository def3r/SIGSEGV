import pandas as pd
from sklearn.model_selection import train_test_split

df = pd.read_excel("Heart.xlsx")
print(df.shape, "\n")

print(df.isnull().sum(), "\n")

print(df.dtypes, "\n")

print((df == 0).sum(), "\n")

print(df['Age'].mean(), "\n")

selected_df = df[['Age', 'Sex', 'ChestPain', 'RestBP', 'Chol']]
train_df, test_df = train_test_split(selected_df, test_size=0.25, random_state=42)
print(train_df.shape)
print(test_df.shape)
