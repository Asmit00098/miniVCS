import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "SecureVCS — Cloud-Native Version Control",
  description:
    "A secure, cloud-native version control system with delta compression, multi-backend BYOS storage, AI-powered merge resolution, and local Ollama search. Your data. Your storage.",
  keywords: ["version control", "S3", "VCS", "git", "secure", "cloud", "C++"],
  openGraph: {
    title: "SecureVCS",
    description: "Cloud-native VCS with BYOS, delta compression, and AI merge resolution.",
    type: "website",
  },
};

export default function RootLayout({
  children,
}: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="en">
      <head>
        <link rel="preconnect" href="https://fonts.googleapis.com" />
        <link rel="preconnect" href="https://fonts.gstatic.com" crossOrigin="anonymous" />
      </head>
      <body>
        {children}
      </body>
    </html>
  );
}
